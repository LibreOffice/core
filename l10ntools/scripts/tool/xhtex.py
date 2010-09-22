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

from l10ntool import AbstractL10nTool
from sdf import SdfEntity
import sys
import xml.dom.minidom

class Xhtex(AbstractL10nTool):
    _resource_type = "xht" 
    _sdfdata       = ()
    _lang          = ""
   
    # Extract methods
    def extract_topic(self, list, inputfile):
        topics = []
        for elem in list:                        
            if elem.childNodes[0].nodeType == elem.TEXT_NODE and len(elem.childNodes[0].data.strip()):
                topics.append(self.prepare_sdf_line(id=elem.getAttribute("id").strip(), text=elem.childNodes[0].data, inputfile=inputfile))            
        return topics
            
    def extract_title(self, list, inputfile):
        titles = []
        for elem in list:
            if len(elem.getAttribute("title").strip()):
                titles.append(self.prepare_sdf_line(id=elem.getAttribute("id").strip(), text=elem.getAttribute("title").strip(), inputfile=inputfile))
        return titles
    
    # Merge methods
    def merge_topic(self, list, sdfdata, lang, inputfilename, dom):
        for elem in list:
            if elem.childNodes[0].nodeType == elem.TEXT_NODE and elem.getAttribute("id").strip():
                obj = self.prepare_sdf_line(inputfile=inputfilename, lang=lang, id=elem.getAttribute("id").strip())
                if sdfdata[obj.get_id()]:
                    elem.childNodes[0].data = str(sdfdata[obj.get_id()].text)  
                
    def merge_title(self, list, sdfdata, lang, inputfilename):
        for elem in list:
            obj = self.prepare_sdf_line(inputfile=inputfilename, lang=lang, id=elem.getAttribute("id").strip())
            if elem.getAttribute("id").strip() and sdfdata[obj.get_id()]: 
                elem.setAttribute("title", str(sdfdata[obj.get_id()].text))

    # L10N tool       
    def __init__(self):
        AbstractL10nTool.__init__(self)
        
    def parse_file(self, filename):
        document = ""
        try:
            f = open(filename, "r+")
            document = f.read()
        except IOError:
            print "ERROR: Can not read file " + filename
            sys.exit(-1)
        else:
            f.close()
        return xml.dom.minidom.parseString(document)

        
    def merge_file(self, inputfilename, outputfilename, parsed_file_ref, lang,is_forced_lang, sdfdata):
        if lang == "en-US":             
            mod_outputfilename = outputfilename.replace("_en-US",'')            
            self.make_dirs(mod_outputfilename)
            self.copy_file(inputfilename, mod_outputfilename)
            return
        dom = parsed_file_ref.cloneNode(True)
        #dom = self.parse_file(inputfilename)    # in case cloneNode is buggy just parse it always
                
        self.merge_topic(dom.getElementsByTagName("topic"), sdfdata, lang, inputfilename, dom)
        self.merge_title(dom.getElementsByTagName("node"), sdfdata, lang, inputfilename)
        self.merge_title(dom.getElementsByTagName("help_section"), sdfdata, lang, inputfilename)
        self.make_dirs(outputfilename)
        try:
            f = open(outputfilename, "w+")
            str = dom.toxml()
            f.write(str)
        except IOError:
            print "ERROR: Can not write file " + outputfilename
            sys.exit(-1)
        else:
            f.close()
    
    ##### Helper for parse-once-use-often like parsing a xml file is needed implement it here
    def parse_file(self, filename):
        document = ""
        try:
            f = open(filename,"r")
            document = f.read()
        except IOError:
            print "ERROR: Can not read file " + filename
        else:
            f.close()        
        return xml.dom.minidom.parseString(document)
    
    ##### Extract a single File
    def extract_file(self, inputfile):
        sdf_data = []
        dom = self.parse_file(inputfile)        
        sdf_data.extend(self.extract_topic(dom.getElementsByTagName("topic"), inputfile))
        sdf_data.extend(self.extract_title(dom.getElementsByTagName("help_section"), inputfile))
        sdf_data.extend(self.extract_title(dom.getElementsByTagName("node"), inputfile))        
        return ''.join([str(line)+"\n" for line in sdf_data])                      

    def prepare_sdf_line(self, inputfile="", lang="" , id="" , text=""):
        if lang == "":
            lang = self._source_language
        return SdfEntity(project=self._options.project_name, source_file=self.get_filename_string(inputfile),
                         resource_type=self._resource_type, gid=id, lid="", langid=lang,text=text)
        
run = Xhtex()
 
