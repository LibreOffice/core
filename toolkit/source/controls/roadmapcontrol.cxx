/*************************************************************************
 *
 *  $RCSfile: roadmapcontrol.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bc $ $Date: 2003-10-06 15:46:42 $
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

#ifndef TOOLKIT_FORMATTED_CONTROL_HXX
#include <toolkit/controls/formattedcontrol.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    // ===================================================================
    // = UnoControlFormattedFieldModel
    // ===================================================================
    // -------------------------------------------------------------------
    UnoControlFormattedFieldModel::UnoControlFormattedFieldModel()
    {
        ImplRegisterProperty( BASEPROPERTY_ALIGN );
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_DEFAULT );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_VALUE );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MAX );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MIN );
        ImplRegisterProperty( BASEPROPERTY_ENABLED );
        ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
        ImplRegisterProperty( BASEPROPERTY_FORMATKEY );
        ImplRegisterProperty( BASEPROPERTY_FORMATSSUPPLIER );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_READONLY );
        ImplRegisterProperty( BASEPROPERTY_SPIN );
        ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
        ImplRegisterProperty( BASEPROPERTY_TEXT );
        ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );

        Any aTreatAsNumber;
        aTreatAsNumber <<= (sal_Bool) sal_True;
        ImplRegisterProperty( BASEPROPERTY_TREATASNUMBER, aTreatAsNumber );
    }

    // -------------------------------------------------------------------
    ::rtl::OUString UnoControlFormattedFieldModel::getServiceName() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel );
    }

    // -------------------------------------------------------------------
    sal_Bool UnoControlFormattedFieldModel::convertFastPropertyValue(
                Any& rConvertedValue, Any& rOldValue, sal_Int32 nPropId,
                const Any& rValue ) throw (IllegalArgumentException)
    {
        if ( BASEPROPERTY_EFFECTIVE_DEFAULT == nPropId )
        {
            sal_Bool bStreamed;
            double dVal = 0;
            sal_Int32  nVal = 0;
            ::rtl::OUString sVal;
            if ( bStreamed = (rValue >>= dVal) )
                rConvertedValue <<= dVal;
            else if ( bStreamed = (rValue >>= nVal) )
                rConvertedValue <<= static_cast<double>(nVal);
            else if ( bStreamed = (rValue >>= sVal) )
                rConvertedValue <<= sVal;

            if ( bStreamed )
            {
                getFastPropertyValue( rOldValue, nPropId );
                return !CompareProperties( rConvertedValue, rOldValue );
            }

            throw IllegalArgumentException(
                        ( ::rtl::OUString::createFromAscii("Unable to convert the given value for the property ")
                    +=  GetPropertyName((sal_uInt16)nPropId) )
                    +=  ::rtl::OUString::createFromAscii(" (double, integer, or string expected)."),
                static_cast< XPropertySet* >(this),
                1);
        }

        return UnoControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nPropId, rValue );
    }

    // -------------------------------------------------------------------
    Any UnoControlFormattedFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        Any aReturn;
        switch (nPropId)
        {
            case BASEPROPERTY_DEFAULTCONTROL: aReturn <<= ::rtl::OUString( ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ) ); break;

            case BASEPROPERTY_TREATASNUMBER: aReturn <<= (sal_Bool)sal_True; break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_FORMATKEY:
            case BASEPROPERTY_FORMATSSUPPLIER:
                // (void)
                break;

            default : aReturn = UnoControlModel::ImplGetDefaultValue( nPropId ); break;
        }

        return aReturn;
    }

    // -------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoControlFormattedFieldModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    // beans::XMultiPropertySet
    // -------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoControlFormattedFieldModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    // ===================================================================
    // = UnoFormattedFieldControl
    // ===================================================================
    // -------------------------------------------------------------------
    UnoFormattedFieldControl::UnoFormattedFieldControl()
    {
    }

    // -------------------------------------------------------------------
    ::rtl::OUString UnoFormattedFieldControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "FormattedField" );
    }

    // -------------------------------------------------------------------
    void UnoFormattedFieldControl::textChanged(const TextEvent& e) throw(RuntimeException)
    {
        Reference< XVclWindowPeer >  xPeer(getPeer(), UNO_QUERY);
        OSL_ENSURE(xPeer.is(), "UnoFormattedFieldControl::textChanged : what kind of peer do I have ?");
        ::rtl::OUString sEffectiveValue = GetPropertyName( BASEPROPERTY_EFFECTIVE_VALUE );
        ImplSetPropertyValue( sEffectiveValue, xPeer->getProperty( sEffectiveValue ), sal_False );

        if ( GetTextListeners().getLength() )
            GetTextListeners().textChanged( e );
    }

//........................................................................
}   // namespace toolkit
//........................................................................
