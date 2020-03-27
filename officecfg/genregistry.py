#!/usr/bin/env python3

import sys, os, subprocess, tempfile, shutil
import xml.etree.ElementTree as ET

srcdir = sys.argv[1]
srcroot = os.path.normpath(os.path.join(srcdir, '..'))
outname = sys.argv[2]
infiles = sys.argv[3:]

xsltproc = shutil.which('xsltproc')
schemaroot = os.path.join(srcroot, 'officecfg/registry/schema')
registrydir = os.path.join(srcroot, 'officecfg/registry')
xcsdir = os.path.join(srcroot, 'org/openoffice/Office/UI') # WRONG

'''S=/mnt/scratch/libreoffice && I=$S/instdir && W=$S/workdir &&
 mkdir -p $W/XcuResTarget/registry/en-US/org/openoffice/Office/UI/ &&
 xsltproc --nonet -o $W/XcuResTarget/registry/en-US/org/openoffice/Office/UI/Effects.xcu
 --stringparam xcs
 $W/XcsTarget/org/openoffice/Office/UI/Effects.xcs
 --stringparam schemaRoot  $S/officecfg/registry/schema
--stringparam locale en-US
--stringparam LIBO_SHARE_FOLDER share
--stringparam LIBO_SHARE_HELP_FOLDER help
--path $S/officecfg/registry $S/officecfg/util/alllang.xsl
 $S/officecfg/registry/data/org/openoffice/Office/UI/Effects.xcu'''

l = '''S=/mnt/scratch/libreoffice && I=$S/instdir && W=$S/workdir &&
 find $W/XcuResTarget/registry/en-US/
  $W/XcuResTarget/driver_calc/en-US/
 $W/XcuResTarget/driver_dbase/en-US/
  $W/XcuResTarget/driver_flat/en-US/
 $W/XcuResTarget/driver_odbc/en-US/
 $W/XcuResTarget/driver_mysql_jdbc/en-US/
 $W/XcuResTarget/driver_mork/en-US/
$W/XcuResTarget/driver_hsqldb/en-US/
 $W/XcuResTarget/driver_jdbc/en-US/
 $W/XcuResTarget/driver_firebird_sdbc/en-US/
  $W/XcuResTarget/driver_mysqlc/en-US/
   $W/XcuResTarget/driver_writer/en-US/
 $W/XcuResTarget/driver_mysql_jdbc/en-US/
 $W/XcuResTarget/driver_postgresql/en-US/
 -name *.xcu | LC_ALL=C sort |
 awk 'BEGIN{print "<list>"} {print "<filename>"$0"</filename>"} END  {print "</list>"}' > $W/CustomTarget/postprocess/registry/registry_en-US.list
'''

final = '''S=/mnt/scratch/libreoffice &&
I=$S/instdir &&
 W=$S/workdir &&
mkdir -p $W/XcdTarget/ &&
xsltproc
--nonet -o
$W/XcdTarget/registry_en-US.xcd
 $S/solenv/bin/packregistry.xslt
$W/CustomTarget/postprocess/registry/registry_en-US.list
'''

with tempfile.TemporaryDirectory() as tmpdir:
    xcus = []
    for i in infiles:
        ofilename = os.path.split(i)[1]
        xcsname = os.path.splitext(ofilename)[0] + '.xcs'
        ofilename_abs = os.path.join(tmpdir, ofilename)
        rc = subprocess.call([xsltproc,
                              '--nonet',
                              '-o',
                              ofilename_abs,
                              '--stringparam',
                              'xcs',
                              xcsname,
                              '--stringparam',
                              'schemaRoot',
                              schemaroot,
                              '--stringparam',
                              'locale',
                              'en-US',
                              '--stringparam',
                              'LIBO_SHARE_FOLDER',
                              'share',
                              '--stringparam',
                              'LIBO_SHARE_HELP_FOLDER',
                              'help',
                              '--path',
                              registrydir,
                              os.path.join(srcroot, 'officecfg/util/alllang.xsl'),
                              i,
                              ])
        if rc != 0:
            sys.exit(rc)
        xcus.append(ofilename_abs)
    root = ET.Element('list')
    for xcu in sorted(xcus):
        n = ET.SubElement(root, 'filename')
        n.text = xcu
    tree = ET.ElementTree(root)
    tmp_xml = os.path.join(tmpdir, 'registry_en-US.list')
    tree.write(tmp_xml, xml_declaration=True)
    rc = subprocess.call([xsltproc,
                          '--nonet',
                          '-o',
                          outname,
                          os.path.join(srcroot, 'solenv/bin/packregistry.xslt'),
                          tmp_xml,
                          ])
    if rc != 0:
        sys.exit(rc)
