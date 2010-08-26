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

from pseudo import PseudoSet,PseudoOrderedDict
from time import gmtime, strftime

class SdfData:
    _filename        = "";
    _dict            = PseudoOrderedDict()
    _languages_found = [];
    
    def __init__ (self, filename=""):
        self._filename = filename
    
    def __getitem__(self, key):
        if self._dict.has_key(key):
            return self._dict[key]
        else:
            return None
    
    def has_key(self, key):
        return self._dict.has_key(key)
    
    def __setitem__(self, key, value):
        self._dict[key] = value
    
    def get_languages_found_in_sdf(self):
        return PseudoSet(self._languages_found)

    def read(self):
        try:
            f = open(self._filename, "r")
            lines = [line.rstrip('\n') for line in f.readlines()] 
        except IOError:
            print "ERROR: Trying to read "+ self._filename
            raise
        else:
            f.close()
        for line in lines:        
            entity = SdfEntity()
            entity.set_properties(line)
            self._dict[entity.get_id()] = entity
            self._languages_found.append(entity.langid)

    def write(self, filename):
        try:
            f = open(filename, "w+") 
            for value in self._dict.itervalues():
                #f.write( repr(value)+"\n" )
                f.write(value + "\n")
        except IOError:
            print "ERROR: Trying to write " + filename
            raise
        else:
            f.close()

import sys
class SdfEntity: 
    # Sdf format columns
    project         = ""
    source_file     = ""
    dummy1          = ""
    resource_type   = ""
    gid             = ""
    lid             = ""
    helpid          = ""
    platform        = ""
    dummy2          = ""
    langid          = ""
    text            = ""
    helptext        = ""
    quickhelptext   = ""
    title           = ""
    date            = ""
    
    import const
    const._PROJECT_POS         = 0
    const._SOURCE_FILE_POS     = 1
    const._DUMMY1_POS          = 2
    const._RESOURCE_TYPE_POS   = 3
    const._GID_POS             = 4
    const._LID_POS             = 5
    const._HELPID_POS          = 6
    const._PLATFORM_POS        = 7
    const._DUMMY2_POS          = 8
    const._LANGID_POS          = 9
    const._TEXT_POS            = 10
    const._HELPTEXT_POS        = 11
    const._QUICKHELPTEXT_POS   = 12
    const._TITLE_POS           = 13
    const._DATE_POS            = 14
        
    def __init__(self, project="", source_file="", dummy1="0", resource_type="", gid="", lid="", helpid="", platform="", dummy2="0", langid="", 
                       text="", helptext="", quickhelptext="", title="", date=""):
        self.project        = project;
        self.source_file    = source_file;
        self.dummy1         = dummy1;
        self.resource_type  = resource_type;
        self.gid            = gid;
        self.lid            = lid;
        self.helpid         = helpid;
        self.platform       = platform;
        self.dummy2         = dummy2;
        self.langid         = langid;
        self.text           = text;
        self.helptext       = helptext;
        self.quickhelptext  = quickhelptext;
        self.title          = title;
        if date != "":
            self.date = date;
        else:
            self.date = strftime("%Y-%m-%d %H:%M:%S",gmtime())


    def set_properties(self, line):
        splitted = line.split("\t")
        if len(splitted) == 15:
            self.project        = splitted[ self.const._PROJECT_POS ]             
            self.source_file    = splitted[ self.const._SOURCE_FILE_POS ]     
            self.dummy1         = splitted[ self.const._DUMMY1_POS ]         
            self.resource_type  = splitted[ self.const._RESOURCE_TYPE_POS ] 
            self.gid            = splitted[ self.const._GID_POS ]             
            self.lid            = splitted[ self.const._LID_POS ]             
            self.helpid         = splitted[ self.const._HELPID_POS ]         
            self.platform       = splitted[ self.const._PLATFORM_POS ]         
            self.dummy2         = splitted[ self.const._DUMMY2_POS ]         
            self.langid         = splitted[ self.const._LANGID_POS ]         
            self.text           = splitted[ self.const._TEXT_POS ]             
            self.helptext       = splitted[ self.const._HELPTEXT_POS ]         
            self.quickhelptext  = splitted[ self.const._QUICKHELPTEXT_POS ] 
            self.title          = splitted[ self.const._TITLE_POS ]         
            self.date           = splitted[ self.const._DATE_POS ]            

    def get_file_id(self):
        return self.project + "\\" + self.source_file
    
    def get_resource_path(self):
            return self.source_file[0:self.source_file.rfind( "\\" )-1]
    
    def __str__(self):
        return ''.join([self.project, "\t", self.source_file, "\t", self.dummy1, "\t", self.resource_type, "\t" , 
            self.gid, "\t", self.lid, "\t", self.helpid, "\t", self.platform, "\t", self.dummy2, "\t" , self.langid, 
            "\t", self.text, "\t", self.helptext, "\t", self.quickhelptext, "\t" , self.title, "\t", self.date ])
    
    def get_id(self):
        return ''.join([self.project, self.gid, self.lid, self.source_file, self.resource_type, self.platform, self.helpid, self.langid])
