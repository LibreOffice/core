#!/usr/bin/env python3

import os, sys, subprocess, shutil
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
    'dtrans/util/mcnttype',
    'embeddedobj/util/embobj',
    'eventattacher/source/evtatt',
    'filter/source/config/cache/filterconfig1',
    'filter/source/flash/flash',
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
    'framework/util/fwl',
    'framework/util/fwm',
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
    'scaddins/source/analysis/analysis',
    'scaddins/source/datefunc/date',
    'scaddins/source/pricing/pricing',
    'sd/util/sd',
    'sd/util/sdd',
    'sd/util/sdfilt',
    'sdext/source/presenter/presenter',
    'sdext/source/minimizer/minimizer',
    'sfx2/util/sfx',
    'slideshow/util/slideshow',
    'sot/util/sot',
    'starmath/util/sm',
    'starmath/util/smd',
    'svl/source/fsstor/fsstorage',
    'svl/source/passwordcontainer/passwordcontainer',
    'svl/util/svl',
    'svtools/util/svt',
    'svgio/svgio',
    'emfio/emfio',
    'svx/util/svx',
    'svx/util/svxcore',
    'svx/util/textconversiondlgs',
    'sw/util/msword',
    'sw/util/sw',
    'sw/util/swd',
    'toolkit/util/tk',
    'ucb/source/sorter/srtrs1',
    'ucb/source/core/ucb1',
    'ucb/source/cacher/cached1',
    'ucb/source/ucp/expand/ucpexpand1',
    'ucb/source/ucp/ext/ucpext',
    'ucb/source/ucp/file/ucpfile1',
    'ucb/source/ucp/hierarchy/ucphier1',
    'ucb/source/ucp/image/ucpimage',
    'ucb/source/ucp/package/ucppkg1',
    'ucb/source/ucp/tdoc/ucptdoc1',
    'UnoControls/util/ctl',
    'unotools/util/utl',
    'unoxml/source/rdf/unordf',
    'unoxml/source/service/unoxml',
    'uui/util/uui',
    'vcl/vcl.common',
    'xmloff/source/transform/xof',
    'xmloff/util/xo',
    'xmlscript/util/xmlscript',
    'sccomp/source/solver/swarmsolver',
    'writerfilter/util/writerfilter',
    'writerperfect/source/draw/wpftdraw',
    'writerperfect/source/impress/wpftimpress',
    'writerperfect/source/writer/wpftwriter',
    'writerperfect/source/calc/wpftcalc',
    'basctl/util/basctl',
    'basic/util/sb',
    'sc/util/vbaobj',
    'scripting/source/basprov/basprov',
    'scripting/source/dlgprov/dlgprov',
    'scripting/source/protocolhandler/protocolhandler',
    'scripting/source/stringresource/stringresource',
    'scripting/source/vbaevents/vbaevents',
    'scripting/util/scriptframe',
    'sw/util/vbaswobj',
    'vbahelper/util/msforms',
    'canvas/source/cairo/cairocanvas',
    'shell/source/sessioninstall/losessioninstall',
    'ucb/source/ucp/gio/ucpgio',
]

tmpdir = os.path.join(os.getcwd(), 'fobba')
if os.path.isdir(tmpdir):
    shutil.rmtree(tmpdir)
os.mkdir(tmpdir)

root = ET.Element('list')

for c in components:
    cbase = os.path.basename(c)
    iname = os.path.join('..', c + '.component')
    oname = os.path.join(tmpdir, cbase + '.component')
    n = ET.SubElement(root, 'filename')
    n.text = oname
    component = ET.parse(iname)
    croot = component.getroot()
    assert('environment' in croot.attrib)
    croot.attrib['environment'] = 'gcc3'
    croot.attrib['uri'] = 'vnd.sun.star.expand:$LO_LIB_DIR/libanimcorelo.so'
    new_root = ET.Element('components')
    new_root.append(component.getroot())
    new_root.attrib['xmlns'] = 'http://openoffice.org/2010/uno-components'
    assert(not os.path.exists(oname))
    tree = ET.ElementTree(new_root)
    tree.write(oname, xml_declaration=True)

tree = ET.ElementTree(root)
tmp_xml = 'servicelist.xml'
tree.write(tmp_xml, xml_declaration=True)

rc = subprocess.call(['xsltproc',
                      '-v',
                      '--nonet',
                      '-o',
                      output_name,
                      input_name,
                      tmp_xml,
])

if rc != 0:
    sys.exit(rc)

os.unlink(tmp_xml)
