/*************************************************************************
 *
 *  $RCSfile: FieldDescriptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:47:10 $
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

namespace dbaui
{
    class OFieldDescription
    {
    private:
        const OTypeInfo*    m_pType;

        ::rtl::OUString     m_sName;
        ::rtl::OUString     m_sTypeName;
        ::rtl::OUString     m_sDescription;
        ::rtl::OUString     m_sDefaultValue;
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
        OFieldDescription(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xAffectedCol);
        OFieldDescription(  const ::rtl::OUString&  _sName,
                            const ::rtl::OUString&  _sTypeName,
                            const ::rtl::OUString&  _sDescription,
                            const ::rtl::OUString&  _sDefaultValue,
                            const OTypeInfo*        _pType,
                            sal_Int32               _nPrecision,
                            sal_Int32               _nScale,
                            sal_Int32               _nIsNullable,
                            sal_Int32               _nFormatKey,
                            SvxCellHorJustify       _eHorJustify,
                            sal_Bool                _bIsAutoIncrement,
                            sal_Bool                _bIsPrimaryKey,
                            sal_Bool                _bIsCurrency)    :
         m_sName(_sName)
        ,m_sTypeName(_sTypeName)
        ,m_sDescription(_sDescription)
        ,m_sDefaultValue(_sDefaultValue)
        ,m_pType(_pType)
        ,m_nPrecision(_nPrecision)
        ,m_nScale(_nScale)
        ,m_nIsNullable(_nIsNullable)
        ,m_nFormatKey(_nFormatKey)
        ,m_eHorJustify(_eHorJustify)
        ,m_bIsAutoIncrement(_bIsAutoIncrement)
        ,m_bIsPrimaryKey(_bIsPrimaryKey)
        ,m_bIsCurrency(_bIsCurrency)
        {
        }
        OFieldDescription( const OFieldDescription& rDescr );
        virtual ~OFieldDescription();

        void SetName(const ::rtl::OUString& _rName)                 { m_sName = _rName; }
        //  void SetTypeName(const ::rtl::OUString& _rTypeName)         { m_sTypeName = _rTypeName; }
        void SetDescription(const ::rtl::OUString& _rDescription)   { m_sDescription = _rDescription; }
        void SetDefaultValue(const ::rtl::OUString& _rDefaultValue) { m_sDefaultValue = _rDefaultValue; }
        void SetType(const OTypeInfo* _pType)                       { m_pType = _pType; }
        void SetTypeValue(sal_Int32 _nType)                         { m_nType = _nType; OSL_ENSURE(!m_pType,"Invalid call here!");}
        void SetPrecision(const sal_Int32& _rPrecision)             { m_nPrecision = _rPrecision; }
        void SetScale(const sal_Int32& _rScale)                     { m_nScale = _rScale; }
        void SetIsNullable(const sal_Int32& _rIsNullable)           { m_nIsNullable = _rIsNullable; }
        void SetFormatKey(const sal_Int32& _rFormatKey)             { m_nFormatKey = _rFormatKey; }
        void SetHorJustify(const SvxCellHorJustify& _rHorJustify)   { m_eHorJustify = _rHorJustify; }
        void SetAutoIncrement(sal_Bool _bAuto)                      { m_bIsAutoIncrement = _bAuto; }
        void SetPrimaryKey(sal_Bool _bPKey)                         { m_bIsPrimaryKey = _bPKey; }
        void SetCurrency(sal_Bool _bIsCurrency)                     { m_bIsCurrency = _bIsCurrency; }

        ::rtl::OUString     GetName()           const { return m_sName; }
        //  ::rtl::OUString     GetTypeName()       const { return m_sTypeName; }
        ::rtl::OUString     GetDescription()    const { return m_sDescription; }
        ::rtl::OUString     GetDefaultValue()   const { return m_sDefaultValue; }
        sal_Int32           GetType()           const { return m_pType ? m_pType->nType : m_nType; }
        sal_Int32           GetPrecision()      const { return m_nPrecision; }
        sal_Int32           GetScale()          const { return m_nScale; }
        sal_Int32           GetIsNullable()     const { return m_nIsNullable; }
        sal_Int32           GetFormatKey()      const { return m_nFormatKey; }
        SvxCellHorJustify   GetHorJustify()     const { return m_eHorJustify; }
        const OTypeInfo*    getTypeInfo()       const { return m_pType; }
        sal_Bool            IsAutoIncrement()   const { return m_bIsAutoIncrement; }
        sal_Bool            IsPrimaryKey()      const { return m_bIsPrimaryKey; }
        sal_Bool            IsCurrency()        const { return m_bIsCurrency; }
        sal_Bool            IsNullable()        const { return m_nIsNullable == ::com::sun::star::sdbc::ColumnValue::NULLABLE; }
    };
}
#endif // DBAUI_FIELDDESCRIPTIONS_HXX


