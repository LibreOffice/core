/*************************************************************************
 *
 *  $RCSfile: srchcfg.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2001-03-20 10:27:54 $
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
#ifndef _SVX_SRCHCFG_HXX
#define _SVX_SRCHCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

//-----------------------------------------------------------------------------
struct SvxSearchConfig_Impl;
struct SvxSearchEngineData
{
    rtl::OUString   sEngineName;

    rtl::OUString   sAndPrefix;
    rtl::OUString   sAndSuffix;
    rtl::OUString   sAndSeparator;
    sal_Int16       nAndCaseMatch;

    rtl::OUString   sOrPrefix;
    rtl::OUString   sOrSuffix;
    rtl::OUString   sOrSeparator;
    sal_Int16       nOrCaseMatch;

    rtl::OUString   sExactPrefix;
    rtl::OUString   sExactSuffix;
    rtl::OUString   sExactSeparator;
    sal_Int16       nExactCaseMatch;

    SvxSearchEngineData() :
        nAndCaseMatch(0),
        nOrCaseMatch(0),
        nExactCaseMatch(0){}

    sal_Bool operator==(const SvxSearchEngineData& rData);
};
class SvxSearchConfig : public utl::ConfigItem
{
    SvxSearchConfig_Impl* pImpl;

public:
    SvxSearchConfig(sal_Bool bEnableNotify = sal_True);
    virtual ~SvxSearchConfig();

    void            Load();
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    sal_uInt16                  Count();
    const SvxSearchEngineData&  GetData(sal_uInt16 nPos);
    const SvxSearchEngineData*  GetData(const rtl::OUString& rEngineName);
    void                        SetData(const SvxSearchEngineData& rData);
    void                        RemoveData(const rtl::OUString& rEngineName);
};

#endif

