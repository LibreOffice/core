/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <vector>
#include <string>

#include "inireader.hxx"

namespace transex3{

class Treeconfig
{

    private:
        INIreader   inireader;
        INImap      map;
        bool        has_config_file;
        void getCurrentDir( string& dir );
        bool isConfigFilePresent();

    public:

        Treeconfig() : has_config_file( false ) { parseConfig(); }
        // read the config file, returns true in case a config file had been found
        bool parseConfig();
        // returns a string vector containing all active  repositories, returns true in case we are deep inside
        // of a source tree. This could affect the behavour of the tool
        bool getActiveRepositories( vector<string>& active_repos);
};

}
