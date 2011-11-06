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



#ifndef BIBMOD_HXX
#define BIBMOD_HXX


class ResMgr;
class BibDataManager;
class BibConfig;

#define BIBLIOGRAPHY_INI_GROUP      "Bibliography"
#define BIBLIOGRAPHY_INI_CUR_NAME   "BibliographyCurrent"
#define BIBLIOGRAPHY_INI_DB_ENTRY   "Bibliography"
#define BIBLIOGRAPHY_INI_MAPPING    "BibliographyFields"
#define BIBLIOGRAPHY_TYPE_NAMES     "BibliographyTypeNames"

class BibModul
{
    private:
        ResMgr*                 pResMgr;
        static BibConfig*       pBibConfig;

    public:
                                BibModul();
                                ~BibModul();

        ResMgr*                 GetResMgr(){return pResMgr;}
        static BibConfig*       GetConfig();

        BibDataManager*         createDataManager();

};

typedef BibModul*       PtrBibModul;
typedef PtrBibModul*    HdlBibModul;

HdlBibModul     OpenBibModul();
void            CloseBibModul(HdlBibModul ppBibModul);

#endif
