/*************************************************************************
 *
 *  $RCSfile: configitem.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:39:44 $
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
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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
#define CONFIG_MODE_IMMEDIATE_UPDATE    0x00
#define CONFIG_MODE_DELAYED_UPDATE      0x01
#define CONFIG_MODE_ALL_LOCALES         0x02
#define CONFIG_MODE_RELEASE_TREE        0x04
#define CONFIG_MODE_IGNORE_ERRORS       0x08 // prevent assertions, if creation fails
#define CONFIG_MODE_PROPAGATE_ERRORS    0x10 // throw exceptions, if creation fails

    enum  ConfigNameFormat
    {
        CONFIG_NAME_PLAINTEXT_NAME, // unescaped local node name, for user display etc.
        CONFIG_NAME_LOCAL_NAME,     // local node name, for use in XNameAccess etc. ("Item", "Q & A")
        CONFIG_NAME_LOCAL_PATH,     // one-level relative path, for use when building pathes etc.  ("Item", "Typ['Q &amp; A']")
        CONFIG_NAME_FULL_PATH,       // full absolute path. ("/org.openoffice.Sample/Group/Item", "/org.openoffice.Sample/Set/Typ['Q &amp; A']")

        CONFIG_NAME_DEFAULT = CONFIG_NAME_LOCAL_PATH // default format
    };

    class ConfigChangeListener_Impl;
    class ConfigManager;
    struct ConfigItem_Impl;

    class ConfigItem
    {
            friend class ConfigChangeListener_Impl;
            friend class ConfigManager;

            const rtl::OUString         sSubTree;
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        m_xHierarchyAccess;
            com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
                                        xChangeLstnr;
            ConfigItem_Impl*            pImpl;

            ConfigItem();//
            void                    RemoveListener();
            void                    CallNotify(
                                const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

            //***********************************************************************************************************************
            // In special mode ALL_LOCALES we must support reading/writing of localized cfg entries as Sequence< PropertyValue >.
            // These methods are helper to convert given lists of names and Any-values.
            // format:  PropertyValue.Name  = <locale as ISO string>
            //          PropertyValue.Value = <value; type depends from cfg entry!>
            // e.g.
            //          LOCALIZED NODE
            //          "UIName"
            //                      LOCALE      VALUE
            //                      "de"        "Mein Name"
            //                      "en-US"     "my name"
            void impl_packLocalizedProperties   (   const   com::sun::star::uno::Sequence< rtl::OUString >&             lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );
            void impl_unpackLocalizedProperties (   const   com::sun::star::uno::Sequence< rtl::OUString >&             lInNames    ,
                                                    const   com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lInValues   ,
                                                            com::sun::star::uno::Sequence< rtl::OUString >&             lOutNames   ,
                                                            com::sun::star::uno::Sequence< com::sun::star::uno::Any >&  lOutValues  );

            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                                        GetTree();

        protected:
            ConfigItem(const rtl::OUString rSubTree,
                        sal_Int16 nMode = CONFIG_MODE_DELAYED_UPDATE);
            ConfigItem(utl::ConfigManager&  rManager, const rtl::OUString rSubTree);

            void                    SetModified  (); // mark item as modified
            void                    ClearModified(); // reset state after commit!

            com::sun::star::uno::Sequence< com::sun::star::uno::Any>
                                    GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            com::sun::star::uno::Sequence< sal_Bool >
                                    GetReadOnlyStates(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);

            sal_Bool                PutProperties(
                                        const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues);

#if SUPD<637
            sal_Bool                EnableNotification(const com::sun::star::uno::Sequence< rtl::OUString >& rNames);
            sal_Bool                EnableNotification(const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        sal_Bool bEnableInternalNotification);
#else
            sal_Bool                EnableNotification(const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                                        sal_Bool bEnableInternalNotification = sal_False);
#endif
            sal_Bool                IsInternalNotification()const {return IsInValueChange();}

            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< rtl::OUString >
                                    GetNodeNames(const rtl::OUString& rNode);
            //returns all members of a node in a specific format
            com::sun::star::uno::Sequence< rtl::OUString >
                                    GetNodeNames(const rtl::OUString& rNode, ConfigNameFormat eFormat);
            // remove all members of a set
            sal_Bool                ClearNodeSet(const rtl::OUString& rNode);
            // remove selected members of a set
            sal_Bool                ClearNodeElements(const rtl::OUString& rNode,
                                        com::sun::star::uno::Sequence< rtl::OUString >& rElements);
            // change or add members to a set
            sal_Bool                SetSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // remove, change or add members of a set
            sal_Bool                ReplaceSetProperties(const rtl::OUString& rNode, com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > rValues);
            // add a new node without setting any properties
            sal_Bool                AddNode(const rtl::OUString& rNode, const rtl::OUString& rNewNode);

            /** get a name for a new element of a set node
                @param _rSetNode
                    relative path to the set node
                @param _rBaseName
                    <ul><li><b>in</b>: the base to use when calculating a new name</li>
                        <li><b>out</b>: an unused element name</li>
                    </ul>
                @return <TRUE/> if a free name could be generated
            */
            sal_Bool                getUniqueSetElementName( const rtl::OUString& _rSetNode, rtl::OUString& _rBaseName);

        public:
            virtual ~ConfigItem();

            /** is called from the ConfigManager before application ends of from the
                PropertyChangeListener if the sub tree broadcasts changes. */
            virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

            /** is called from the ConfigManager if it is destroyed before the ConfigItem. */
            void                    ReleaseConfigMgr();

            /** enable locking of the XHierarchicalNameAccess if CONFIG_MODE_RELEASE_TREE is set to
             prevent multiple calls ConfigManager::AcquireTree() from a single Commit() operation*/
            void                    LockTree();
            void                    UnlockTree();

            const rtl::OUString&    GetSubTreeName() const {return sSubTree;}

            sal_Bool                IsModified() const;

            /** writes the changed values into the sub tree. Always called in the Dtor of the derived class.  */
            virtual void            Commit();

            sal_Bool                IsInValueChange() const;

            sal_Int16               GetMode() const;

            /** checks if the configuration manager used by this item is valid.
            */
            sal_Bool                IsValidConfigMgr() const;
    };
}//namespace utl
#endif //_UTL_CONFIGITEM_HXX_
