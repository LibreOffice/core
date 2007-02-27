/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optutil.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:57:02 $
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

#ifndef SC_OPTUTIL_HXX
#define SC_OPTUTIL_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif


class ScOptionsUtil
{
public:
    static BOOL     IsMetricSystem();
};


//  ConfigItem for classes that use items from several sub trees

class ScLinkConfigItem : public utl::ConfigItem
{
    Link    aCommitLink;

public:
            ScLinkConfigItem( const rtl::OUString rSubTree );
            ScLinkConfigItem( const rtl::OUString rSubTree, sal_Int16 nMode );
    void    SetCommitLink( const Link& rLink );

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();

    void    SetModified()   { ConfigItem::SetModified(); }
    com::sun::star::uno::Sequence< com::sun::star::uno::Any>
            GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames)
                            { return ConfigItem::GetProperties( rNames ); }
    sal_Bool PutProperties( const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                            const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues)
                            { return ConfigItem::PutProperties( rNames, rValues ); }

    using ConfigItem::EnableNotification;
    using ConfigItem::GetNodeNames;

//  sal_Bool EnableNotification(com::sun::star::uno::Sequence< rtl::OUString >& rNames)
//                          { return ConfigItem::EnableNotification( rNames ); }

//  com::sun::star::uno::Sequence< rtl::OUString > GetNodeNames(rtl::OUString& rNode)
//                          { return ConfigItem::GetNodeNames( rNode ); }
};

#endif


