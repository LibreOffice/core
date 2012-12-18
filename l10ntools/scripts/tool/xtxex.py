#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

from l10ntool import AbstractL10nTool
from sdf import SdfEntity
import sys
import shutil

class Xtxex(AbstractL10nTool):
    _resource_type       = "xtx"

    def __init__(self):
        AbstractL10nTool.__init__(self)

    def merge_file(self, inputfilename, outputfilename, parsed_file_ref, lang, is_forced_lang, sdfdata):
        # Special handling for en-US files
        if lang == "en-US":
            mod_outputfilename = outputfilename
            # mod here if needed
            self.copy_file(inputfilename, mod_outputfilename)
            return
        # merge usual lang
        sdfline = self.prepare_sdf_line(inputfilename,lang)
        if sdfline.get_id() in sdfdata:
            line = sdfdata[sdfline.get_id()].text.replace("\\n", '\n')
            self.make_dirs(outputfilename)
            try:
                f = open(outputfilename, "w+")
                f.write(line)
            except IOError:
                print("ERROR: Can not write file " + outputfilename)
                sys.exit(-1)
            else:
                f.close()
            return
        # no sdf data found then copy en-US source file
        if is_forced_lang:
            self.copy_file(inputfilename, outputfilename)

    ##### Extract a single File
    def extract_file(self, inputfile):
        lines = []
        try:
            f = open(inputfile, "r")
            lines = f.readlines()
        except IOError:
            print("ERROR: Can not open file " + inputfile)
            sys.exit(-1)
        else:
            f.close()
        # remove legal header
        lines = [line for line in lines if len(line) > 0 and not line[0] == '#']
        # escape all returns
        lines = [line.replace('\n', "\\n") for line in lines]
        line = ''.join(lines)
        test = str(line)
        if len(test.strip()):
            sdf_entity = self.prepare_sdf_line(inputfile);
            sdf_entity.text = line
            return str(sdf_entity)
        else:
            return ""

    def prepare_sdf_line(self, inputfile="", lang=""):
        if lang == "":
            lang = self._source_language
        return SdfEntity(project=self._options.project_name, source_file=self.get_filename_string(inputfile),
                          resource_type=self._resource_type, gid="none", lid="none", langid=lang,text="")

run = Xtxex()
