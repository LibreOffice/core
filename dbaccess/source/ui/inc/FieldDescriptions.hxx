/*************************************************************************
 *
 *  $RCSfile: FieldDescriptions.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:54:46 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#define DBAUI_FIELDDESCRIPTIONS_HXX

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

namespace dbaui
{
    class OFieldDescription

    {
    private:
        ::com::sun::star::uno::Any      m_aDefaultValue;    // the default value from the database
        ::com::sun::star::uno::Any      m_aControlDefault;  // the value which the control inserts as default

        TOTypeInfoSP    m_pType;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDestInfo;

        ::rtl::OUString     m_sName;
        ::rtl::OUString     m_sTypeName;
        ::rtl::OUString     m_sDescription;

        ::rtl::OUString     m_sAutoIncrementValue;
        sal_Int32           m_nType;    // only used when m_pType is null
        sal_Int32           m_nPrecision;
        sal_Int32           m_nScale;
        sal_Int32           m_nIsNullable;
        sal_Int32           m_nFormatKey;
        SvxCellHorJustify   m_eHorJustify;
        sal_Bool            m_bIsAutoIncrement;
        sal_Bool            m_bIsPrimaryKey;
        sal_Bool            m_bIsCurrency;

    public:
        OFieldDescription();
        OFieldDescription(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xAffectedCol
                         ,sal_Bool _bUseAsDest = sal_False);
        OFieldDescription(  const ::rtl::OUString&  _sName,
                            const ::rtl::OUString&  _sTypeName,
                            const ::rtl::OUString&  _sDescription,
                            const ::com::sun::star::uno::Any&   _aDefaultValue,
                            const ::com::sun::star::uno::Any&   _aControlDefault,
                            const ::rtl::OUString&  _sAutoIncrementValue,
                            const TOTypeInfoSP&     _pType,
                            sal_Int32               _nPrecision,
                            sal_Int32               _nScale,
                            sal_Int32               _nIsNullable,
                            sal_Int32               _nFormatKey,
                            SvxCellHorJustify       _eHorJustify,
                            sal_Bool                _bIsAutoIncrement,
                            sal_Bool                _bIsPrimaryKey,
                            sal_Bool                _bIsCurrency);
        OFieldDescription( const OFieldDescription& rDescr );
        ~OFieldDescription();

        void SetName(const ::rtl::OUString& _rName);
        void SetDescription(const ::rtl::OUString& _rDescription);
        void SetDefaultValue(const ::com::sun::star::uno::Any& _rDefaultValue);
        void SetControlDefault(const ::com::sun::star::uno::Any& _rControlDefault);
        void SetAutoIncrementValue(const ::rtl::OUString& _sAutoIncValue);
        void SetType(TOTypeInfoSP _pType);
        void SetTypeValue(sal_Int32 _nType);
        void SetPrecision(const sal_Int32& _rPrecision);
        void SetScale(const sal_Int32& _rScale);
        void SetIsNullable(const sal_Int32& _rIsNullable);
        void SetFormatKey(const sal_Int32& _rFormatKey);
        void SetHorJustify(const SvxCellHorJustify& _rHorJustify);
        void SetAutoIncrement(sal_Bool _bAuto);
        void SetPrimaryKey(sal_Bool _bPKey);
        void SetCurrency(sal_Bool _bIsCurrency);

        void FillFromTypeInfo(const TOTypeInfoSP& _pType,sal_Bool _bForce,sal_Bool _bReset);

        ::rtl::OUString             GetName()               const;
        ::rtl::OUString             GetDescription()        const;
        ::com::sun::star::uno::Any  GetDefaultValue()       const;
        ::com::sun::star::uno::Any  GetControlDefault()     const;
        ::rtl::OUString             GetAutoIncrementValue() const;
        sal_Int32                   GetType()               const;
        sal_Int32                   GetPrecision()          const;
        sal_Int32                   GetScale()              const;
        sal_Int32                   GetIsNullable()         const;
        sal_Int32                   GetFormatKey()          const;
        SvxCellHorJustify           GetHorJustify()         const;
        TOTypeInfoSP                getTypeInfo()           const;
        sal_Bool                    IsAutoIncrement()       const;
        sal_Bool                    IsPrimaryKey()          const;
        sal_Bool                    IsCurrency()            const;
        sal_Bool                    IsNullable()            const;
    };
}
#endif // DBAUI_FIELDDESCRIPTIONS_HXX


