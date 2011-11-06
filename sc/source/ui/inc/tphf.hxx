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



#ifndef SC_TPHF_HXX
#define SC_TPHF_HXX


#include <svx/hdft.hxx>

class ScStyleDlg;

//========================================================================

class ScHFPage : public SvxHFPage
{
public:
    virtual         ~ScHFPage();

    virtual void    Reset( const SfxItemSet& rSet );
    virtual sal_Bool    FillItemSet( SfxItemSet& rOutSet );

    void            SetPageStyle( const String& rName )    { aStrPageStyle = rName; }
    void            SetStyleDlg ( const ScStyleDlg* pDlg ) { pStyleDlg = pDlg; }

protected:
                    ScHFPage( Window* pParent,
                              sal_uInt16 nResId,
                              const SfxItemSet& rSet,
                              sal_uInt16 nSetId );

//    using SvxHFPage::ActivatePage;
//    using SvxHFPage::DeactivatePage;
    virtual void    ActivatePage();
    virtual void    DeactivatePage();
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

private:
    PushButton          aBtnEdit;
    SfxItemSet          aDataSet;
    String              aStrPageStyle;
    sal_uInt16              nPageUsage;
    const ScStyleDlg*   pStyleDlg;

#ifdef _TPHF_CXX
private:
    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( HFEditHdl, void* );
    DECL_LINK( TurnOnHdl, CheckBox* );
#endif
};

//========================================================================

class ScHeaderPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    ScHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

//========================================================================

class ScFooterPage : public ScHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    ScFooterPage( Window* pParent, const SfxItemSet& rSet );
};


#endif // SC_TPHF_HXX

