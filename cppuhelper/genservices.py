#!/usr/bin/env python3

import os, sys, subprocess, shutil, tempfile
import xml.etree.ElementTree as ET

output_name = sys.argv[1]
input_name = sys.argv[2]

components = [
    'animations/source/animcore/animcore',
    'avmedia/util/avmedia',
    'chart2/source/controller/chartcontroller',
    'chart2/source/chartcore',
    'canvas/source/factory/canvasfactory',
    'canvas/source/simplecanvas/simplecanvas',
    'canvas/source/vcl/vclcanvas',
    'comphelper/util/comphelp',
    'configmgr/source/configmgr',
    'cppcanvas/source/uno/mtfrenderer',
    'cui/util/cui',
    'desktop/source/deployment/deployment',
    'drawinglayer/drawinglayer',
    'embeddedobj/util/embobj',
    'eventattacher/source/evtatt',
    'filter/source/config/cache/filterconfig1',
    'filter/source/graphic/graphicfilter',
    'filter/source/msfilter/msfilter',
    'filter/source/odfflatxml/odfflatxml',
    'filter/source/pdf/pdffilter',
    'filter/source/storagefilterdetect/storagefd',
    'filter/source/svg/svgfilter',
    'filter/source/t602/t602filter',
    'filter/source/textfilterdetect/textfd',
    'filter/source/xmlfilteradaptor/xmlfa',
    'filter/source/xmlfilterdetect/xmlfd',
    'filter/source/xsltdialog/xsltdlg',
    'filter/source/xsltfilter/xsltfilter',
    'formula/util/for',
    'framework/util/fwk',
    'hwpfilter/source/hwp',
    'i18npool/source/search/i18nsearch',
    'i18npool/util/i18npool',
    'lingucomponent/source/hyphenator/hyphen/hyphen',
    'lingucomponent/source/languageguessing/guesslang',
    'lingucomponent/source/spellcheck/spell/spell',
    'lingucomponent/source/thesaurus/libnth/lnth',
    'lingucomponent/source/numbertext/numbertext',
    'linguistic/source/lng',
    'lotuswordpro/util/lwpfilter',
    'oox/util/oox',
    'package/source/xstor/xstor',
    'package/util/package2',
    'sax/source/expatwrap/expwrap',
    'sc/util/sc',
    'sc/util/scd',
    'sc/util/scfilt',
#    'scaddins/source/analysis/analysis',
#    'scaddins/source/datefunc/date',
#    'scaddins/source/pricing/pricing',
    'sd/util/sd',
#    'sd/util/sdd',
    'sd/util/sdfilt',
#    'sdext/source/presenter/presenter',
#    'sdext/source/minimizer/minimizer',
    'sfx2/util/sfx',
    'slideshow/util/slideshow',
    'sot/util/sot',
    'starmath/util/sm',
    'starmath/util/smd',
    'svl/source/fsstor/fsstorage',
#    'svl/source/passwordcontainer/passwordcontainer',
    'svl/util/svl',
    'svtools/util/svt',
    'svgio/svgio',
    'emfio/emfio',
    'svx/util/svx',
    'svx/util/svxcore',
#    'svx/util/textconversiondlgs',
    'sw/util/msword',
    'sw/util/sw',
#    'sw/util/swd',
    'toolkit/util/tk',
    'ucb/source/sorter/srtrs1',
    'ucb/source/core/ucb1',
#    'ucb/source/cacher/cached1',
    'ucb/source/ucp/expand/ucpexpand1',
#    'ucb/source/ucp/ext/ucpext',
    'ucb/source/ucp/file/ucpfile1',
    'ucb/source/ucp/hierarchy/ucphier1',
#    'ucb/source/ucp/image/ucpimage',
    'ucb/source/ucp/package/ucppkg1',
    'ucb/source/ucp/tdoc/ucptdoc1',
    'UnoControls/util/ctl',
    'unotools/util/utl',
    'unoxml/source/rdf/unordf',
    'unoxml/source/service/unoxml',
    'uui/util/uui',
#    'vcl/vcl.common',
    'xmloff/source/transform/xof',
    'xmloff/util/xo',
    'xmlscript/util/xmlscript',
#    'sccomp/source/solver/swarmsolver',
#    'writerfilter/util/writerfilter',
#    'writerperfect/source/draw/wpftdraw',
#    'writerperfect/source/impress/wpftimpress',
    'writerperfect/source/writer/wpftwriter',
#    'writerperfect/source/calc/wpftcalc',
    'basctl/util/basctl',
    'basic/util/sb',
    'sc/util/vbaobj',
#    'scripting/source/basprov/basprov',
#    'scripting/source/dlgprov/dlgprov',
    'scripting/source/protocolhandler/protocolhandler',
#    'scripting/source/stringresource/stringresource',
#    'scripting/source/vbaevents/vbaevents',
#    'scripting/util/scriptframe',
    'sw/util/vbaswobj',
    'vbahelper/util/msforms',
    'canvas/source/cairo/cairocanvas',
#    'shell/source/sessioninstall/losessioninstall',
#    'ucb/source/ucp/gio/ucpgio',
]

def check_identicality(c):
    import pathlib
    p = pathlib.Path('/mnt/scratch/libreofficemeson/workdir/ComponentTarget')
    l = list(p.glob('**/' + os.path.split(c)[1]))
    assert(len(l) == 1)
    truth = l[0]
    truthcomponent = ET.parse(truth)
    truthcomponent.write('temppi.xml', xml_declaration=True)
    d1 = open(c).read()
    d2 = open('temppi.xml').read()
    if d1 != d2:
        print(c)
        print(d1)
        sys.exit(1)

# This is terrible and hacky and should be replaced with
# introspection or something similar.
def determine_libname(source_root, component_base):
    component_name = os.path.split(component_base)[1]
    component_dir = component_base.split('/', 1)[0]
    meson_file = os.path.join(source_root, component_dir, 'meson.build')
    assert(os.path.exists(meson_file))
    losearch = "shared_library('{}lo'".format(component_name)
    nolosearch = "shared_library('{}'".format(component_name)
    with open(meson_file) as mfile:
        for line in mfile:
            if losearch in line:
                return 'lib{}lo.so'.format(component_name)
            if nolosearch in line:
                return 'lib{}.so'.format(component_name)
    # Haximus Maximux
    if component_name == 'comphelp':
        return 'libcomphelper.so'
    if component_name == 'filterconfig1':
        return 'libfilterconfiglo.so'
    if component_name == 'lwpfilter':
        return 'liblwpft.so'
    sys.exit('Could not find shared library for {}.'.format(component_base))

with tempfile.TemporaryDirectory() as tmpdir:
    root = ET.Element('list')

    for c in components:
        cbase = os.path.basename(c)
        iname = os.path.join('..', c + '.component')
        oname = os.path.join(tmpdir, cbase + '.component')
        assert 'xmlns' in open(iname).read()
        assert(not os.path.exists(oname))
        n = ET.SubElement(root, 'filename')
        n.text = oname
        component = ET.parse(iname)
        libname = determine_libname('..', c)
        croot = component.getroot()
        assert('environment' in croot.attrib)
        croot.attrib['environment'] = 'gcc3'
        croot.attrib['uri'] = 'vnd.sun.star.expand:$LO_LIB_DIR/' + libname
        new_root = ET.Element('components')
        new_root.append(component.getroot())
#        new_root.attrib['xmlns'] = 'http://openoffice.org/2010/uno-components'
        tree = ET.ElementTree(new_root)
        tree.write(oname, xml_declaration=True)
        hack = open(oname).read()
        hack = hack.replace('ns0:', '').replace(':ns0', '')
        open(oname, 'w').write(hack)
        #check_identicality(oname)

    tree = ET.ElementTree(root)
    tmp_xml = os.path.join(tmpdir, 'servicelist.xml')
    tree.write(tmp_xml, xml_declaration=True)

    rc = subprocess.call(['xsltproc',
                          #'-v',
                          '--nonet',
                          '-o',
                          output_name,
                          input_name,
                          tmp_xml,
    ])

    if rc != 0:
        sys.exit(rc)
