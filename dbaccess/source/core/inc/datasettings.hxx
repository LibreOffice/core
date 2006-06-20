/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasettings.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:46:49 $
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
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#include <comphelper/propertystatecontainer.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODataSettings_Base - a base class which implements the property member
//=                 for an object implementing the sdb::DataSettings
//=                 service
//= the properties have to to be registered when used
//==========================================================================
class ODataSettings_Base
{
public:
// <properties>
    ::rtl::OUString                             m_sFilter;
    ::rtl::OUString                             m_sHavingClause;
    ::rtl::OUString                             m_sGroupBy;
    ::rtl::OUString                             m_sOrder;
    sal_Bool                                    m_bApplyFilter;     // no BitField ! the base class needs a pointer to this member !
    ::com::sun::star::awt::FontDescriptor       m_aFont;
    ::com::sun::star::uno::Any                  m_aRowHeight;
    ::com::sun::star::uno::Any                  m_aTextColor;
    ::com::sun::star::uno::Any                  m_aTextLineColor;
    sal_Int16                                   m_nFontEmphasis;
    sal_Int16                                   m_nFontRelief;
// </properties>

protected:
    ODataSettings_Base();
    ODataSettings_Base(const ODataSettings_Base& _rSource);
};
//==========================================================================
//= ODataSettings - a base class which implements the property handling
//=                 for an object implementing the sdb::DataSettings
//=                 service
//==========================================================================

class ODataSettings : public ::comphelper::OPropertyStateContainer
                    , public ODataSettings_Base
{
    sal_Bool m_bQuery;
protected:
    ODataSettings(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bQuery = sal_False);
    ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bQuery = sal_False);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;

    /** register the properties from the param given. The parameter instance must be alive as long as tis object live.
        @param  _pItem
            The database settings, can be <br>this</br>
    */
    void registerPropertiesFor(ODataSettings_Base* _pItem);
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_DATASETTINGS_HXX_

