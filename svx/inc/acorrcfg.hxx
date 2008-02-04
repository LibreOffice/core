/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acorrcfg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 15:37:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVXACCFG_HXX
#define _SVXACCFG_HXX

// include ---------------------------------------------------------------

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
/* -----------------------------12.10.00 11:40--------------------------------

 ---------------------------------------------------------------------------*/
class SvxAutoCorrect;
class SvxAutoCorrCfg;
class SVX_DLLPUBLIC SvxBaseAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

public:
    SvxBaseAutoCorrCfg(SvxAutoCorrCfg& rParent);
    ~SvxBaseAutoCorrCfg();

    void                    Load(sal_Bool bInit);
    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void                    SetModified() {ConfigItem::SetModified();}
};
/* -----------------------------12.10.00 11:40--------------------------------

 ---------------------------------------------------------------------------*/
class SVX_DLLPUBLIC SvxSwAutoCorrCfg : public utl::ConfigItem
{
    SvxAutoCorrCfg& rParent;
    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

public:
    SvxSwAutoCorrCfg(SvxAutoCorrCfg& rParent);
    ~SvxSwAutoCorrCfg();

    void                    Load(sal_Bool bInit);
    virtual void            Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void                    SetModified() {ConfigItem::SetModified();}
};
/*--------------------------------------------------------------------
    Beschreibung:   Konfiguration fuer Auto Correction
 --------------------------------------------------------------------*/
class SVX_DLLPUBLIC SvxAutoCorrCfg
{
    friend class SvxBaseAutoCorrCfg;
    friend class SvxSwAutoCorrCfg;

    SvxAutoCorrect* pAutoCorrect;

    SvxBaseAutoCorrCfg      aBaseConfig;
    SvxSwAutoCorrCfg        aSwConfig;

    // Flags f"ur Autotext:
    sal_Bool    bFileRel;
    sal_Bool    bNetRel;
    // Tiphilfe f"ur Autotext w"ahrend der Eingabe
    sal_Bool    bAutoTextTip;
    sal_Bool    bAutoTextPreview;
    sal_Bool    bAutoFmtByInput;
    sal_Bool    bSearchInAllCategories;

public:
    void        SetModified()
                {
                    aBaseConfig.SetModified();
                    aSwConfig.SetModified();
                }
    void        Commit()
                {
                    aBaseConfig.Commit();
                    aSwConfig.Commit();
                }

          SvxAutoCorrect* GetAutoCorrect()          { return pAutoCorrect; }
    const SvxAutoCorrect* GetAutoCorrect() const    { return pAutoCorrect; }
    // der Pointer geht in den Besitz des ConfigItems!
    void SetAutoCorrect( SvxAutoCorrect* );

    sal_Bool IsAutoFmtByInput() const       { return bAutoFmtByInput; }
    void SetAutoFmtByInput( sal_Bool bSet ) { bAutoFmtByInput = bSet;aSwConfig.SetModified();}

    sal_Bool IsSaveRelFile() const          { return bFileRel; }
    void SetSaveRelFile( sal_Bool bSet )    { bFileRel = bSet; aSwConfig.SetModified(); }

    sal_Bool IsSaveRelNet() const           { return bNetRel; }
    void SetSaveRelNet( sal_Bool bSet )     { bNetRel = bSet; aSwConfig.SetModified();}

    sal_Bool IsAutoTextPreview() const {return bAutoTextPreview;}
    void     SetAutoTextPreview(sal_Bool bSet) {bAutoTextPreview = bSet; aSwConfig.SetModified();}

    sal_Bool IsAutoTextTip() const          { return bAutoTextTip; }
    void SetAutoTextTip(sal_Bool bSet )     { bAutoTextTip = bSet;aSwConfig.SetModified();}

    sal_Bool IsSearchInAllCategories() const        { return bSearchInAllCategories;}
    void SetSearchInAllCategories(sal_Bool bSet )   { bSearchInAllCategories = bSet; aSwConfig.SetModified(); }

    SvxAutoCorrCfg();
    virtual ~SvxAutoCorrCfg();
    static SvxAutoCorrCfg*      Get();
};


#endif

