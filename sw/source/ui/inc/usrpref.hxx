/*************************************************************************
 *
 *  $RCSfile: usrpref.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2001-02-13 09:52:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _USRPREF_HXX
#define _USRPREF_HXX



#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#include "viewopt.hxx"

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

/* -----------------------------28.09.00 09:45--------------------------------

 ---------------------------------------------------------------------------*/
class SwMasterUsrPref;
class SwContentViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    BOOL                    bWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwContentViewConfig(BOOL bWeb, SwMasterUsrPref& rParent);
        ~SwContentViewConfig();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};
/* -----------------------------28.09.00 09:45--------------------------------

 ---------------------------------------------------------------------------*/
class SwLayoutViewConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;
    BOOL                bWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwLayoutViewConfig(BOOL bWeb, SwMasterUsrPref& rParent);
        ~SwLayoutViewConfig();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};
/* -----------------------------19.01.01 13:06--------------------------------

 ---------------------------------------------------------------------------*/
class SwGridConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;
    BOOL                bWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwGridConfig(BOOL bWeb, SwMasterUsrPref& rParent);
        ~SwGridConfig();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};
/* -----------------------------19.01.01 13:06--------------------------------

 ---------------------------------------------------------------------------*/
class SwCursorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&    rParent;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
    public:
        SwCursorConfig(SwMasterUsrPref& rParent);
        ~SwCursorConfig();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};
/* -----------------------------28.09.00 09:45--------------------------------

 ---------------------------------------------------------------------------*/
class SwWebColorConfig : public utl::ConfigItem
{
    SwMasterUsrPref&        rParent;
    com::sun::star::uno::Sequence<rtl::OUString> aPropNames;

    public:
        SwWebColorConfig(SwMasterUsrPref& rParent);
        ~SwWebColorConfig();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();
    void                    SetModified(){ConfigItem::SetModified();}
};
/* -----------------------------28.09.00 09:45--------------------------------

 ---------------------------------------------------------------------------*/
class SwMasterUsrPref : public SwViewOption
{
    friend class SwContentViewConfig;
    friend class SwLayoutViewConfig;
    friend class SwGridConfig;
    friend class SwCursorConfig;
    friend class SwWebColorConfig;

    SwContentViewConfig aContentConfig;
    SwLayoutViewConfig  aLayoutConfig;
    SwGridConfig        aGridConfig;
    SwCursorConfig      aCursorConfig;
    SwWebColorConfig*   pWebColorConfig;

    sal_Int32   nFldUpdateFlags;    //udpate of fields and charts
    sal_Bool    bFldUpdateInCurrDoc;
    sal_Int32   nLinkUpdateMode;
    sal_Bool    bLinkUpdateInCurrDoc;
    FieldUnit   eUserMetric;

    sal_Int32   nDefTab;            //default tab stop distance

public:
    SwMasterUsrPref(BOOL bWeb);
    ~SwMasterUsrPref();

    void SetUsrPref(const SwViewOption &rCopy);

    void Commit()
        {
            aContentConfig.Commit();
            aLayoutConfig.Commit();
            aGridConfig.Commit();
            aCursorConfig.Commit();
            if(pWebColorConfig)
                pWebColorConfig->Commit();
        }
    void SetModified()
        {
            aContentConfig.SetModified();
            aLayoutConfig.SetModified();
            aGridConfig.SetModified();
            aCursorConfig.SetModified();
            if(pWebColorConfig)
                pWebColorConfig->SetModified();
        }

    void SetUpdateLinkMode(sal_Int32 nSet)  {nLinkUpdateMode = nSet; SetModified();}
    sal_Int32 GetUpdateLinkMode() const {return nLinkUpdateMode; }

    sal_Bool    IsUpdateLinksToCurrDoc() const {return bLinkUpdateInCurrDoc;}
    void        SetUpdateLinksToCurrDoc(sal_Bool bSet){ bLinkUpdateInCurrDoc = bSet; SetModified();}

    sal_Bool    IsUpdateFieldsToCurrDoc() const {return bFldUpdateInCurrDoc;}
    void        SetUpdateFieldsToCurrDoc(sal_Bool bSet){ bFldUpdateInCurrDoc = bSet; SetModified();}


    void SetUpdateFields(BOOL bSet)
        {
            if(bSet && nFldUpdateFlags == AUTOUPD_OFF)
            {
                nFldUpdateFlags = AUTOUPD_FIELD_ONLY;
                SetModified();
             }
            else if(!bSet)
            {
                nFldUpdateFlags = AUTOUPD_OFF;
                SetModified();
            }
        };
    sal_Bool IsUpdateFields()const {return nFldUpdateFlags != AUTOUPD_OFF; }

    sal_Int32   GetFldUpdateFlags()const {return nFldUpdateFlags;}
    void        SetFldUpdateFlags(sal_Int32 nSet){nFldUpdateFlags = nSet;}

    void SetUpdateCharts(BOOL bSet)
        {
            if(bSet)
            {
                nFldUpdateFlags = AUTOUPD_FIELD_AND_CHARTS;
                SetModified();
             }
             else if(nFldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS)
             {
                nFldUpdateFlags = AUTOUPD_FIELD_ONLY;
                SetModified();
             }
        };
    sal_Bool IsUpdateCharts()const {return nFldUpdateFlags == AUTOUPD_FIELD_AND_CHARTS; }

    FieldUnit   GetMetric() const { return eUserMetric;}
    void        SetMetric(FieldUnit eSet) { eUserMetric = eSet; SetModified();};

    sal_Int32   GetDefTab() const { return nDefTab;}
    void        SetDefTab( sal_Int32  nSet ) {  nDefTab = nSet; SetModified();}

};

#endif

