/*************************************************************************
 *
 *  $RCSfile: configitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-11-20 11:37:46 $
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

#ifndef _UTL_CONFIGITEM_HXX_
#define _UTL_CONFIGITEM_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
//-----------------------------------------------------------------------------
namespace com{ namespace sun{ namespace star{
    namespace uno{
        class Any;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace container{
        class XHierarchicalNameAccess;
    }
    namespace util{
        class XChangesListener;
    }
}}}
//-----------------------------------------------------------------------------
namespace utl
{
    class ConfigChangeListener_Impl;
    class ConfigManager;
    class ConfigItem
    {
            friend class ConfigChangeListener_Impl;

            const rtl::OUString         sSubTree;
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        xHierarchyAccess;
            com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
                                        xChangeLstnr;
            utl::ConfigManager*         pManager;
            sal_Bool                    bIsModified             : 1; //
            sal_Bool                    bInPutValues            : 1; //prevent notification of own change actions
            sal_Bool                    bHasChangedProperties   : 1; //call XChangesBatch::Commit() if any changes were notified

            ConfigItem();//
            void                    RemoveListener();
            void                    CallNotify(
                                const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

        protected:
            ConfigItem(const rtl::OUString rSubTree);
            ConfigItem(utl::ConfigManager&  rManager, const rtl::OUString rSubTree);

            void                    SetModified() {bIsModified = sal_True;}

            com::sun::star::uno::Sequence< com::sun::star::uno::Any>
                                    GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            sal_Bool                PutProperties(
                                        const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues);

            sal_Bool                EnableNotification(com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            //returns all members of a node
            com::sun::star::uno::Sequence< rtl::OUString >
                                    GetNodeNames(const rtl::OUString& rNode);
            // remove all members of a set
            sal_Bool                ClearNodeSet(const rtl::OUString& rNode);
            // change or add members to a set
            sal_Bool                SetSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // remove, change or add members of a set
            sal_Bool                ReplaceSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);

        public:
            virtual ~ConfigItem();

            /** is called from the ConfigManager before application ends of from the
                 PropertyChangeListener if the sub tree broadcasts changes. */
            virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

            /** is called from the ConfigManager if it is destroyed before the ConfigItem. */
            void                    ReleaseConfigMgr();

            const rtl::OUString&    GetSubTreeName() const {return sSubTree;}

            sal_Bool                IsModified() const {return bIsModified;}

            /** writes the changed values into the sub tree. Always called in the Dtor of the derived class.  */
            virtual void            Commit();

    };
}//namespace utl
#endif //_UTL_CONFIGITEM_HXX_
