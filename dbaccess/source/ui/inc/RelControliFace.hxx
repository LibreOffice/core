/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelControliFace.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:12:55 $
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
#ifndef DBAUI_RELCONTROLIFACE_HXX
#define DBAUI_RELCONTROLIFACE_HXX

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

namespace dbaui
{
    class OTableConnectionData;
    class IRelationControlInterface
    {
    public:
        virtual ~IRelationControlInterface(){}

        /** getConnectionData returns the current connection data
            @return the current connectiondata
        */
        virtual OTableConnectionData* getConnectionData() const = 0;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(sal_Bool _bValid) = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() = 0;

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange() = 0;
    };
}
#endif // DBAUI_RELCONTROLIFACE_HXX
