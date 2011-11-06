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



#ifndef SC_TEXTDLGS_HXX
#define SC_TEXTDLGS_HXX

#include <sfx2/tabdlg.hxx>

class SfxObjectShell;

//
//      Zeichen-Tab-Dialog
//

class ScCharDlg : public SfxTabDialog
{
private:
    const SfxItemSet&       rOutAttrs;
    const SfxObjectShell&   rDocShell;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
            ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                        const SfxObjectShell* pDocShell );
            ~ScCharDlg() {}
};

//
//      Absatz-Tab-Dialog
//

class ScParagraphDlg : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
            ScParagraphDlg( Window* pParent, const SfxItemSet* pAttr );
            ~ScParagraphDlg() {}
};



#endif

