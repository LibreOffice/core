/*************************************************************************
 *
 *  $RCSfile: javaoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2001-05-18 13:09:47 $
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

#pragma hdrstop

#ifndef _SVTOOLS_JAVAPTIONS_HXX
#include <javaoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define C2U(cChar) OUString::createFromAscii(cChar)
/* -----------------------------10.04.01 12:39--------------------------------

 ---------------------------------------------------------------------------*/
class SvtExecAppletsItem_Impl : public utl::ConfigItem
{
    sal_Bool  bExecute;
public:
    SvtExecAppletsItem_Impl();

    virtual void    Commit();

    sal_Bool IsExecuteApplets() const {return bExecute;}
    void     SetExecuteApplets(sal_Bool bSet) {bExecute = bSet;}
};
/* -----------------------------18.05.01 14:44--------------------------------

 ---------------------------------------------------------------------------*/
SvtExecAppletsItem_Impl::SvtExecAppletsItem_Impl() :
        utl::ConfigItem(C2U("Office.Common/Java/Applet")),
        bExecute(sal_False)
{
    Sequence< OUString > aNames(1);
    aNames.getArray()[0] = C2U("Enable");
    Sequence< Any > aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(aValues.getLength() && pValues[0].hasValue())
        bExecute = *(sal_Bool*)pValues[0].getValue();
}
void    SvtExecAppletsItem_Impl::Commit()
{
    Sequence< OUString > aNames(1);
    aNames.getArray()[0] = C2U("Enable");
    Sequence< Any > aValues(1);
    aValues.getArray()[0].setValue(&bExecute, ::getBooleanCppuType());
    PutProperties(aNames, aValues);
}


struct SvtJavaOptions_Impl
{
    SvtExecAppletsItem_Impl aExecItem;
    Sequence<OUString>      aPropertyNames;
    sal_Bool                bEnabled;
    sal_Bool                bSecurity;
    sal_Int32               nNetAccess;
    rtl::OUString           sUserClassPath;

    SvtJavaOptions_Impl() :
        bEnabled(sal_False),
        bSecurity(sal_False),
        nNetAccess(0),
        aPropertyNames(4)
        {
            OUString* pNames = aPropertyNames.getArray();
            pNames[0] = C2U("Enable");
            pNames[1] = C2U("Security");
            pNames[2] = C2U("NetAccess");
            pNames[3] = C2U("UserClassPath");
        }
};
/* -----------------------------18.05.01 13:28--------------------------------

 ---------------------------------------------------------------------------*/
SvtJavaOptions::SvtJavaOptions() :
    utl::ConfigItem(C2U("Office.Java/VirtualMachine")),
    pImpl(new SvtJavaOptions_Impl)
{
    Sequence< Any > aValues = GetProperties(pImpl->aPropertyNames);
    const Any* pValues = aValues.getConstArray();
    if ( aValues.getLength() == pImpl->aPropertyNames.getLength() )
    {
        for ( int nProp = 0; nProp < pImpl->aPropertyNames.getLength(); nProp++ )
        {
            if( pValues[nProp].hasValue() )
            {
                switch ( nProp )
                {
                    case 0: pImpl->bEnabled = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 1: pImpl->bSecurity = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 2: pValues[nProp] >>= pImpl->nNetAccess; break;
                    case 3: pValues[nProp] >>= pImpl->sUserClassPath; break;
                }
            }
        }
    }
}
/* -----------------------------18.05.01 13:28--------------------------------

 ---------------------------------------------------------------------------*/
SvtJavaOptions::~SvtJavaOptions()
{
    delete pImpl;
}
/*-- 18.05.01 13:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvtJavaOptions::Commit()
{
    pImpl->aExecItem.Commit();
    OUString* pNames = pImpl->aPropertyNames.getArray();
    Sequence<Any> aValues(pImpl->aPropertyNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < pImpl->aPropertyNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp].setValue(&pImpl->bEnabled, rType); break;
            case  1: pValues[nProp].setValue(&pImpl->bSecurity, rType);break;
            case  2: pValues[nProp] <<= pImpl->nNetAccess; break;
            case  3: pValues[nProp] <<= pImpl->sUserClassPath; break;
        }
    }
    PutProperties(pImpl->aPropertyNames, aValues);
}
/*-- 18.05.01 13:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool        SvtJavaOptions::IsEnabled() const
{
    return pImpl->bEnabled;
}
/*-- 18.05.01 13:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool        SvtJavaOptions::IsSecurity()const
{
    return pImpl->bSecurity;
}
/*-- 18.05.01 13:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32       SvtJavaOptions::GetNetAccess() const
{
    return pImpl->nNetAccess;
}
/*-- 18.05.01 13:28:36---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString&  SvtJavaOptions::GetUserClassPath()const
{
    return pImpl->sUserClassPath;
}
/*-- 18.05.01 13:28:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtJavaOptions::SetEnabled(sal_Bool bSet)
{
    if(pImpl->bEnabled != bSet)
    {
        pImpl->bEnabled = bSet;
        SetModified();
    }
}
/*-- 18.05.01 13:28:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtJavaOptions::SetSecurity(sal_Bool bSet)
{
    if(pImpl->bSecurity != bSet)
    {
        pImpl->bSecurity = bSet;
        SetModified();
    }
}
/*-- 18.05.01 13:28:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtJavaOptions::SetNetAccess(sal_Int32 nSet)
{
    if(pImpl->nNetAccess != nSet)
    {
        pImpl->nNetAccess = nSet;
        SetModified();
    }
}
/*-- 18.05.01 13:28:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtJavaOptions::SetUserClassPath(const rtl::OUString& rSet)
{
    if(pImpl->sUserClassPath != rSet)
    {
        pImpl->sUserClassPath = rSet;
        SetModified();
    }
}

/*-- 18.05.01 14:34:32---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool        SvtJavaOptions::IsExecuteApplets() const
{
    return pImpl->aExecItem.IsExecuteApplets();
}
/*-- 18.05.01 14:34:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtJavaOptions::SetExecuteApplets(sal_Bool bSet)
{
    if(pImpl->aExecItem.IsExecuteApplets() != bSet)
    {
        pImpl->aExecItem.SetExecuteApplets(bSet);
        SetModified();
    }
}




