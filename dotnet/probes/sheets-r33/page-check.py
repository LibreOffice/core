#!/usr/bin/env python3
"""Compare our page rectangle, per sheet, against LibreOffice's own flat-ODF page layout:
paper size, the four margins, the scale, and the header/footer band heights. Together with
geom-check.py's column widths these are every input to ScTable::UpdatePageBreaks along the
column axis, so agreement on both means our column bands cannot differ from LibreOffice's."""
import re, sys, subprocess

PROBE = ('/home/user/libreoffice-core/.claude/worktrees/sheets-r33/dotnet/probes/'
         'sheets-r33/GridProbe/bin/Debug/net10.0/linux-x64/GridProbe')
UNIT = {'in': 1440.0, 'cm': 1440/2.54, 'mm': 144/2.54, 'pt': 20.0}


def tw(text):
    m = re.match(r'([\d.]+)(in|cm|mm|pt)$', text)
    return round(float(m.group(1)) * UNIT[m.group(2)]) if m else None


def unescape(name):
    return (name.replace('&amp;', '&').replace('&lt;', '<')
                .replace('&gt;', '>').replace('&quot;', '"').replace('&apos;', "'"))


def reference(path):
    s = open(path, encoding='utf-8').read()
    layouts = {}
    for m in re.finditer(r'<style:page-layout style:name="([^"]+)">(.*?)</style:page-layout>', s, re.S):
        body = m.group(2)
        props = re.search(r'<style:page-layout-properties([^>]*)>', body).group(1)

        def attr(name):
            a = re.search(name + r'="([^"]*)"', props)
            return tw(a.group(1)) if a else None

        band = {}
        for which, tag in (('hdr', 'header'), ('ftr', 'footer')):
            t = re.search(r'<style:%s-style>\s*<style:header-footer-properties([^>]*)/>' % tag, body)
            if not t:
                band[which] = None
                continue
            a = t.group(1)
            fixed = re.search(r'svg:height="([^"]*)"', a)
            minimum = re.search(r'fo:min-height="([^"]*)"', a)
            gap = re.search(r'fo:margin-%s="([^"]*)"' % ('bottom' if which == 'hdr' else 'top'), a)
            band[which] = ('fixed' if fixed else 'dynamic',
                           tw((fixed or minimum).group(1)) if (fixed or minimum) else None,
                           tw(gap.group(1)) if gap else None)
        scale = re.search(r'style:scale-to="(\d+)%"', props)
        layouts[m.group(1)] = dict(
            w=attr('fo:page-width'), h=attr('fo:page-height'),
            l=attr('fo:margin-left'), r=attr('fo:margin-right'),
            t=attr('fo:margin-top'), b=attr('fo:margin-bottom'),
            scale=int(scale.group(1)) if scale else None, **band)

    masters = {m.group(1): m.group(2) for m in
               re.finditer(r'<style:master-page style:name="([^"]+)"[^>]*style:page-layout-name="([^"]+)"', s)}
    styles = {m.group(1): m.group(2) for m in
              re.finditer(r'<style:style style:name="(ta\d+)"[^>]*?style:master-page-name="([^"]*)"', s)}
    sheets = {}
    for m in re.finditer(r'<table:table table:name="([^"]*)" table:style-name="(ta\d+)"', s):
        master = styles.get(m.group(2))
        sheets[unescape(m.group(1))] = layouts.get(masters.get(master, ''), {})
    return sheets


if __name__ == '__main__':
    out = subprocess.run([PROBE, sys.argv[1], '', '0', '0'], capture_output=True, text=True).stdout
    ref = reference(sys.argv[2])
    name = None
    for line in out.splitlines():
        m = re.match(r"# sheet '(.*)' hidden=", line)
        if m:
            name = m.group(1)
            continue
        m = re.match(r'  page=(\d+)x(\d+) L(\d+) R(\d+) T(\d+) B(\d+) hdr(\d+) ftr(\d+) '
                     r'scale=(\w+)/(\d+)', line)
        if not m or name is None:
            continue
        w, h, l, r, t, b, hdr, ftr = (int(m.group(i)) for i in range(1, 9))
        them = ref.get(name)
        if not them:
            print(f'  {name!r}: no page layout in the export')
            continue
        bad = []
        for label, mine, theirs in (('w', w, them['w']), ('h', h, them['h']),
                                    ('L', l, them['l']), ('R', r, them['r'])):
            if theirs is not None and abs(mine - theirs) > 1:
                bad.append(f'{label} {mine} vs {theirs}')
        # top/bottom: ours already folds the band in, theirs states the two separately
        for label, mine, mar, band in (('T', t, them['t'], them['hdr']),
                                       ('B', b, them['b'], them['ftr'])):
            if mar is not None and abs(mine - mar) > 1:
                bad.append(f'{label} {mine} vs {mar}')
        for label, mine, theirs in (('hdr', hdr, them['hdr']), ('ftr', ftr, them['ftr'])):
            if theirs and theirs[0] == 'fixed' and theirs[1] is not None and abs(mine - theirs[1]) > 1:
                bad.append(f'{label} {mine} vs pinned {theirs[1]}')
        if them['scale'] is not None and int(m.group(10)) != them['scale'] \
                and m.group(9) == 'Percentage':
            bad.append(f"scale {m.group(10)} vs {them['scale']}")
        print(('  OK   ' if not bad else '  DIFF ') + repr(name)
              + ('' if not bad else '  ' + '; '.join(bad)))
