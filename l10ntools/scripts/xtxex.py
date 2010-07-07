#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

from l10ntool import abstractL10nTool
from sdf import SdfEntity
import sys
import shutil 

class xtxex(abstractL10nTool):
    _resourceType       = "xtx" 
       
    def __init__(self):
        abstractL10nTool.__init__(self)
        
    def merge_file(self, inputfilename, outputfilename, parsed_file_ref, lang,is_forced_lang, sdfdata):
        print "merge_file lang " + lang +" file " + outputfilename
        sdfline = self.prepare_sdf_line(inputfilename,lang)
        if sdfdata.has_key(sdfline.get_id()):
            line = sdfdata[sdfline.get_id()].text.replace("\\n", '\n')
            self.make_dirs(outputfilename)
            try:
                f = open(outputfilename, "w+")
                f.write(line)
            except IOError:
                print "ERROR: Can not write file " + outputfilename
                sys.exit(-1)
            else:
                f.close()
            return
        if is_forced_lang:
            try:
                shutil.copy(inputfilename, outputfilename)
            except IOError:
                print "ERROR: Can not copy file '" + inputfilename + "' to " + "'" + outputfilename + "'"
                sys.exit(-1)
            
    ##### Extract a single File
    def extract_file(self, inputfile):
        lines = []
        try:
            f = open(inputfile, "r")
            lines = f.readlines()
        except IOError:
            print "ERROR: Can not open file " + inputfile
            sys.exit(-1)
        else:
            f.close()
        # remove legal header
        lines = [line for line in lines if len(line) > 0 and not line[0] == '#']        
        # escape all returns
        lines = [line.replace('\n', "\\n") for line in lines]
        line = ''.join(lines)
        sdf_entity = self.prepare_sdf_line(inputfile);
        sdf_entity.text = line
        return str(sdf_entity)
    
    ##### Nameing scheme for the output files
    def get_outputfile_format_str(self):
        # filename,fileNoExt,language,extension,pathPrefix,pathPostFix,path
        return "{path}/{fileNoExt}_{language}.{extension}"

    def prepare_sdf_line(self, inputfile="", lang=""):
        if lang == "":
            lang = self._source_language
        return SdfEntity(project=self._options.project_name, source_file=self.get_filename_string(inputfile),
                          resource_type=self._resource_type, gid="none", lid="none", langid=lang,text="")
         
run = xtxex()
