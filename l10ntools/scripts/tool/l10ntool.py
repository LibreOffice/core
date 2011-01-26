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

from optparse import OptionParser
from sdf import SdfData
from pseudo import PseudoSet

import sys
import os
import shutil

class AbstractL10nTool:
    _options            = {}
    _args               = ""
    _resource_type      = "" 
    _source_language    = "en-US"
   
    ##### Implement these abstract methods

    ##### Nameing scheme for the output files
    def get_outputfile_format_str(self):
        # filename,fileNoExt,language,extension,pathPrefix,pathPostFix,path
        #return "{path}/{fileNoExt}_{language}.{extension}"
        return self._options.pattern

    ################################# Merge single files ###########################################

    ##### Merge a single file
    def merge_file(self, inputfilename, outputfilename, parsed_file_ref, lang, is_forced_lang, sdfdata):
        pass

    ##### Helper for parse-once-use-often like parsing a xml file is needed implement it here
    def parse_file(self, filename):
        return None

    ################### Merge one big file containing all strings in all languages #################
    def merge_one_big_file(self, inputfile, outputfilename, parsed_file_ref, lang, sdfdata):
        pass

    ################### Extract a single File ######################################################
    def extract_file(self, inputfile):
        pass
    
    ################################################################################################
     
    def format_outputfile(self, filename, language):
        extension = filename[filename.rfind('.')+1:]
        file = filename[:filename.rfind('.')]
        # Python 2.3.x friendly
        return self.get_outputfile_format_str().replace('[', '%(').replace(']',')s') % \
                { 'filename': filename, 'fileNoExt': file, 'language': language, 'extension': extension, 'path_prefix': self._options.path_prefix,
                  'path_postfix': self._options.path_postfix, 'path': self.get_path() }

        #return self.get_outputfile_format_str().replace('[', '{').replace(']','}').format(
        #       filename=filename, fileNoExt=file, language=language, extension=extension, path_prefix=self._options.path_prefix,
        #       path_postfix=self._options.path_postfix, path=self.get_path())

    def get_path(self):
        if self._options.outputfile.find('/') == -1:
            return ""
        else:
            return self._options.outputfile[:self._options.outputfile.rfind('/')]
            
    def merge(self,  sdfdata):
        langset,forcedset, foundset = PseudoSet(), PseudoSet() , PseudoSet()

        if self._options.languages:       
            langset = PseudoSet(self._options.languages)  
        if self._options.forcedlanguages: 
            forcedset = PseudoSet(self._options.forcedlanguages) 
        if sdfdata.get_languages_found_in_sdf(): 
            foundset = sdfdata.get_languages_found_in_sdf() 
    
        if self.has_multi_inputfiles(): 
            filelist = self.read_inputfile_list()
        else:
            filelist = self._options.inputfile
            
        for inputfile in filelist:
            ref = self.parse_file(inputfile)
            # Don't write that files if there is no l10n present
            if ((langset & foundset) - forcedset):  # all langs given and found in sdf without enforced 
                [self.merge_file(inputfile,self.format_outputfile(inputfile, lang), ref, lang, False, sdfdata) for lang in ((langset & foundset) - forcedset)]
            # Always write those files even if there is no l10n available
            if forcedset: # all enforced langs
                [self.merge_file(inputfile, self.format_outputfile(inputfile, lang), ref, lang, True, sdfdata)  for lang in forcedset]
            # In case a big file have to be written
            if ((langset & foundset) | forcedset): # all langs given ,found in sdf and enforced ones
                self.merge_one_big_file(inputfile, self.format_outputfile(inputfile, lang), ref, ((langset & foundset) | forcedset), sdfdata)
    
    def has_multi_inputfiles(self): 
        return self._options.inputfile[0] == '@'

    def copy_file(self, inputfilename, outputfilename):
        try: 
            os.remove(outputfilename)
        except:
            pass    

        try:
            shutil.copy(inputfilename, outputfilename)
        except IOError:
            print "ERROR: Can not copy file '" + inputfilename + "' to " + "'" + outputfilename + "'"
            sys.exit(-1)
    
    def extract(self):
        try:
            f = open(self._options.outputfile, "w+")
            f.write(self.extract_file(self._options.inputfile))
        except IOError:
            print "ERROR: Can not write file " + self._options.outputfile
        else:
            f.close()
            
    # Parse the common options
    def parse_options(self):
        parser = OptionParser()
        parser.add_option("-i", "--inputfile",       dest="inputfile",       metavar="FILE", help="resource file to read"         )
        parser.add_option("-o", "--outputfile",      dest="outputfile",      metavar="FILE", help="extracted sdf or merged file"  )
        parser.add_option("-m", "--inputsdffile",    dest="input_sdf_file",  metavar="FILE", help="merge this sdf file"           )
        parser.add_option("-x", "--pathprefix",      dest="path_prefix",     metavar="PATH", help=""                              )
        parser.add_option("-y", "--pathpostfix",     dest="path_postfix",    metavar="PATH", help=""                              )
        parser.add_option("-p", "--projectname",     dest="project_name",    metavar="NAME", help=""                              )
        parser.add_option("-r", "--projectroot",     dest="project_root",    metavar="PATH", help=""                              )
        parser.add_option("-f", "--forcedlanguages", dest="forcedlanguages", metavar="ISOCODE[,ISOCODE]", help="Always merge those langs even if no l10n is available for those langs" )
        parser.add_option("-l", "--languages",       dest="languages",       metavar="ISOCODE[,ISOCODE]", help="Merge those langs if l10n is found for each")
        parser.add_option("-s", "--pattern",         dest="pattern",         metavar="", help=""                                  )        
        parser.add_option("-q", "--quiet",           action="store_true",    dest="quietmode", help="",default=False)
        (self._options, self.args) = parser.parse_args()
        
        # -l "de,pr,pt-BR" => [ "de" , "pt" , "pt-BR" ]
        parse_complex_arg = lambda arg: arg.split(",")
        
        if self._options.forcedlanguages: 
            self._options.forcedlanguages = parse_complex_arg(self._options.forcedlanguages) 
        if self._options.languages:       
            self._options.languages = parse_complex_arg(self._options.languages) 
        self.test_options()
        
    def __init__(self):
        self.parse_options()
        if self._options.input_sdf_file != None and len(self._options.input_sdf_file):
            sdfdata = SdfData(self._options.input_sdf_file)
            sdfdata.read()
            self.merge(sdfdata)
        else:
            self.extract()

    def make_dirs(self, filename):
        dir = filename[:filename.rfind('/')]
        if os.path.exists(dir):
            if os.path.isfile(dir):
                print "ERROR: There is a file '"+dir+"' where I want create a directory"
                sys.exit(-1)
            else:
                return
        else:
            try:
                os.makedirs(dir)
            except IOError:
                print "Error: Can not create dir " + dir
                sys.exit(-1)
            
    def test_options(self):
        opt = self._options
        is_valid = lambda x: x != None and len(x) > 0
        return  is_valid(opt.project_root) and is_valid(opt.project_name) and is_valid(opt.languages) and \
                ( is_valid(opt.inputfile) and (( is_valid(opt.path_prefix) and is_valid(opt.path_postfix) ) or is_valid(opt.outputfile)) and \
                ( ( is_valid(opt.input_sdf_file) and ( is_valid(opt.outputfile) or  ( is_valid(opt.path_prefix) and is_valid(opt.path_postfix) ) or \
                ( is_valid(opt.inputfile) and is_valid(opt.outputFile)) ))))
        print "Strange options ..."
        sys.exit( -1 )
                     
    def read_inputfile_list(self):
        if self.has_multi_inputfiles():
            lines = []
            try:
                f = open(self._options.inputfile[1:], "r")
                lines = [line.strip('\n') for line in f.readlines()]
            except IOError:
                print "ERROR: Can not read file list " + self._options.inputfile[2:]
                sys.exit(-1)
            else:
                f.close()
            return lines
        
    def get_filename_string(self, inputfile):
        absfile = os.path.realpath(os.path.abspath(inputfile))
        absroot = os.path.realpath(os.path.abspath(self._options.project_root)) 
        return absfile[len(absroot)+1:].replace('/','\\')
    
