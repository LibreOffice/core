/*************************************************************************
 *
 *  $RCSfile: datasettings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 09:37:27 $
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

#ifndef _DBA_CORE_DATASETTINGS_HXX_
#define _DBA_CORE_DATASETTINGS_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif

//==========================================================================
//= ODataSettings_Base - a base class which implements the property member
//=                 for an object implementing the sdb::DataSettings
//=                 service
//= the properties have to to be registered when used
//==========================================================================
class ODataSettings_Base
{
protected:
// <properties>
    ::rtl::OUString                             m_sFilter;
    ::rtl::OUString                             m_sOrder;
    sal_Bool                                    m_bApplyFilter;     // no BitField ! the base class needs a pointer to this member !
    ::com::sun::star::awt::FontDescriptor       m_aFont;
    ::com::sun::star::uno::Any                  m_aRowHeight;
    ::com::sun::star::uno::Any                  m_aTextColor;
// </properties>

protected:
    ODataSettings_Base();
    ODataSettings_Base(const ODataSettings_Base& _rSource);

    /** store all configuration relevant informations under the given configuration node (matching the configuration
        scheme for "DefinitionSettings" - which has yet to be defined :)
        @param      _rxConfigLocation       the configuration node. must not be readonly
    */
    virtual void    storeTo(
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigLocation) const;

    /** load all configuration relevant informations from the given configuration node.
        @param      _rxConfigLocation       the configuration node. must not be readonly
    */
    virtual void    loadFrom(
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigLocation);

};
//==========================================================================
//= ODataSettings - a base class which implements the property handling
//=                 for an object implementing the sdb::DataSettings
//=                 service
//==========================================================================

class ODataSettings : public ::comphelper::OPropertyContainer
                    , public ODataSettings_Base
{
protected:
    ODataSettings(::cppu::OBroadcastHelper& _rBHelper);
    ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper);
private:
    void registerProperties();
};

#endif // _DBA_CORE_DATASETTINGS_HXX_

