/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javaoptions.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-05 17:37:55 $
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
#ifndef _SVTOOLS_JAVAOPTIONS_HXX
#define _SVTOOLS_JAVAOPTIONS_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif


// class SvtJavaOptions --------------------------------------------------

struct SvtJavaOptions_Impl;

class SVL_DLLPUBLIC SvtJavaOptions : public utl::ConfigItem
{
    SvtJavaOptions_Impl* pImpl;
public:
    enum EOption
    {
        E_ENABLED,
        E_SECURITY,
        E_NETACCESS,
        E_USERCLASSPATH,
        E_EXECUTEAPPLETS
    };

    SvtJavaOptions();
    ~SvtJavaOptions();

    virtual void    Commit();

    sal_Bool        IsEnabled() const;
    sal_Bool        IsSecurity()const;
    sal_Int32       GetNetAccess() const;
    rtl::OUString&  GetUserClassPath()const;
    sal_Bool        IsExecuteApplets() const;

    void SetEnabled(sal_Bool bSet) ;
    void SetSecurity(sal_Bool bSet);
    void SetNetAccess(sal_Int32 nSet) ;
    void SetUserClassPath(const rtl::OUString& rSet);
    void SetExecuteApplets(sal_Bool bSet);

    sal_Bool IsReadOnly( EOption eOption ) const;
};

#endif //

