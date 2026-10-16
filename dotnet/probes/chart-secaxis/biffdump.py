#!/usr/bin/env python3
"""Dump a BIFF8 workbook stream's records, indenting chart substreams by CHBEGIN/CHEND."""
import sys, struct, olefile

NAMES = {
 0x1002:'CHCHART',0x1003:'CHSERIES',0x1006:'CHDATAFORMAT',0x1007:'CHLINEFORMAT',
 0x1009:'CHMARKERFORMAT',0x100A:'CHAREAFORMAT',0x100B:'CHPIEFORMAT',0x100C:'CHATTACHEDLABEL',
 0x100D:'CHSTRING',0x1014:'CHTYPEGROUP',0x1015:'CHLEGEND',0x1016:'CHSERIESLIST',0x1017:'CHBAR',
 0x1018:'CHLINE',0x1019:'CHPIE',0x101A:'CHAREA',0x101B:'CHSCATTER',0x101C:'CHCHARTLINE',
 0x101D:'CHAXIS',0x101E:'CHTICK',0x101F:'CHVALUERANGE',0x1020:'CHLABELRANGE',0x1021:'CHAXISLINE',
 0x1022:'CHCHART3D',0x1024:'CHDEFAULTTEXT',0x1025:'CHTEXT',0x1026:'CHFONT',0x1027:'CHOBJECTLINK',
 0x1032:'CHFRAME',0x1033:'CHBEGIN',0x1034:'CHEND',0x1035:'CHPLOTFRAME',0x103A:'CHCHART3DDATAFORMAT',
 0x103C:'CHPICFORMAT',0x103D:'CHDROPBAR',0x103E:'CHRADARLINE',0x103F:'CHSURFACE',0x1040:'CHRADARAREA',
 0x1041:'CHAXESSET',0x1043:'CHLABEL',0x1044:'CHPROPERTIES',0x1045:'CHSERGROUP',0x1046:'CHUSEDAXESSETS',
 0x1048:'CHPIVOTREF',0x104A:'CHSERPARENT',0x104B:'CHSERTRENDLINE',0x104E:'CHFORMAT',
 0x104F:'CHFRAMEPOS',0x1050:'CHFORMATRUNS',0x1051:'CHSOURCELINK',0x105B:'CHSERERRORBAR',
 0x105D:'CHSERIESFORMAT',0x105F:'CH3DDATAFORMAT',0x1060:'CHFBI',0x1061:'CHBOXPROPS',
 0x1062:'CHDATERANGE',0x1063:'CHESCHERFORMAT',0x1064:'CHFRAMEPOS?',0x1065:'CHPIEEXT',
 0x1066:'CHESCHERFORMAT',0x1068:'CHFONTX?',0x1069:'CHUNITS?',0x086B:'?',
 0x0809:'BOF',0x000A:'EOF',0x0200:'DIMENSIONS',0x00EB:'MSODRAWINGGROUP',0x00EC:'MSODRAWING',
 0x005D:'OBJ',0x007F:'IMDATA',0x0093:'STYLE',0x00FC:'SST',0x00FD:'LABELSST',0x0203:'NUMBER',
 0x0204:'LABEL',0x0207:'STRING',0x0006:'FORMULA',0x0085:'BOUNDSHEET',0x0031:'FONT',
 0x041E:'FORMAT',0x00E0:'XF',0x1051:'CHSOURCELINK',
}

def recs(data):
    p = 0
    while p + 4 <= len(data):
        rid, ln = struct.unpack_from('<HH', data, p)
        yield p, rid, data[p+4:p+4+ln]
        p += 4 + ln

def main(path, want_chart_only=True):
    ole = olefile.OleFileIO(path)
    for nm in ('Workbook','Book'):
        if ole.exists(nm):
            data = ole.openstream(nm).read(); break
    depth = 0
    inchart = 0
    for off, rid, body in recs(data):
        nm = NAMES.get(rid, hex(rid))
        if rid == 0x0809 and len(body) >= 4:
            dt = struct.unpack_from('<H', body, 2)[0]
            if dt == 0x0020: inchart = 1
            elif dt in (0x0010,0x0005,0x0040): inchart = 0
        if want_chart_only and not inchart and rid not in (0x0809,):
            continue
        if rid == 0x1034: depth -= 1
        print('%08X %s%-16s len=%3d %s' % (off, '  '*max(depth,0), nm, len(body), body[:48].hex()))
        if rid == 0x1033: depth += 1

if __name__ == '__main__':
    main(sys.argv[1], len(sys.argv) < 3)
