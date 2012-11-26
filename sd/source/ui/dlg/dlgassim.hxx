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



#include <svl/dateitem.hxx>
#include "drawdoc.hxx"
#include <svx/svdotext.hxx>
#include <svtools/svtreebx.hxx>

#include "sdresid.hxx"

class SdPageListControl : public SvTreeListBox
{
private:
    SvLBoxButtonData*   m_pCheckButton;

    SvLBoxEntry* InsertPage( const String& rPageName );
    void InsertTitle( SvLBoxEntry* pEntry, const String& rTitle );

public:
    SdPageListControl( Window* pParent, const ResId& rResId );
    ~SdPageListControl();

    void Fill( SdDrawDocument* pDoc );
    void Clear();

    sal_uInt32 GetSelectedPage();
    bool IsPageChecked( sal_uInt32 nPage );

    DECL_LINK( CheckButtonClickHdl, SvLBoxButtonData * );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

};

class TemplateCacheInfo;

DECLARE_LIST( TemplateCacheInfoList, TemplateCacheInfo * )

class TemplateCacheDirEntry
{
public:
    String                      m_aPath;
    TemplateCacheInfoList       m_aFiles;
};

DECLARE_LIST( TemplateCacheDirEntryList, TemplateCacheDirEntry * )
