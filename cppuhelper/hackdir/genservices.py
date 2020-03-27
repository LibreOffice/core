#!/usr/bin/env python3

import os, sys, subprocess, shutil, tempfile
import xml.etree.ElementTree as ET

output_name = sys.argv[1]
input_name = sys.argv[2]

components = [
  'binaryurp/source/binaryurp',
  'io/source/io',
  'stoc/util/bootstrap',
  'stoc/source/inspect/introspection',
  'stoc/source/invocation_adapterfactory/invocadapt',
  'stoc/source/invocation/invocation',
  'stoc/source/namingservice/namingservice',
  'stoc/source/proxy_factory/proxyfac',
  'stoc/source/corereflection/reflection',
  'stoc/util/stocservices',
  'remotebridges/source/unourl_resolver/uuresolver',
#  'stoc/source/javaloader/javaloader',
#  'stoc/source/javavm/javavm',
]


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
        new_root.attrib['xmlns'] = 'http://openoffice.org/2010/uno-components'
        tree = ET.ElementTree(new_root)
        tree.write(oname, xml_declaration=True)

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
