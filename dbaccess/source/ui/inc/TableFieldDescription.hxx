/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_TABLEFIELDDESC_HXX
#define DBAUI_TABLEFIELDDESC_HXX

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#include "QEnumTypes.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ref.hxx>

#include <salhelper/simplereferenceobject.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

class Window;
namespace dbaui
{
    class OTableFieldDesc : public ::salhelper::SimpleReferenceObject
    {
    private:
       ::std::vector< ::rtl::OUString >
                                    m_aCriteria;

        ::rtl::OUString             m_aTableName;
        ::rtl::OUString             m_aAliasName;       // table range
        ::rtl::OUString             m_aFieldName;       // column
        ::rtl::OUString             m_aFieldAlias;  // column alias
        ::rtl::OUString             m_aFunctionName;    // enth"alt den Funktionsnamen, nur wenn m_eFunctionType != FKT_NONE gesetzt

        Window*                     m_pTabWindow;

        sal_Int32                   m_eDataType;
        sal_Int32                   m_eFunctionType;
        ETableFieldType             m_eFieldType;
        EOrderDir                   m_eOrderDir;
        sal_Int32                   m_nIndex;
        sal_Int32                   m_nColWidth;
        sal_uInt16                  m_nColumnId;
        sal_Bool                    m_bGroupBy;
        sal_Bool                    m_bVisible;

        // !!!! bitte bei Erweiterung um neue Felder auch IsEmpty mitpflegen !!!!

    public:
        OTableFieldDesc();
        OTableFieldDesc(const ::rtl::OUString& rTable, const ::rtl::OUString& rField );
        OTableFieldDesc(const OTableFieldDesc& rRS);
        ~OTableFieldDesc();

        inline sal_Bool         IsEmpty() const;

        OTableFieldDesc&        operator=( const OTableFieldDesc& _aField );
        sal_Bool                operator==( const OTableFieldDesc& rDesc );

        sal_Bool                IsVisible() const    { return m_bVisible;}
        sal_Bool                IsGroupBy() const    { return m_bGroupBy;}

        void                    SetVisible( sal_Bool bVis=sal_True ){ m_bVisible = bVis;}
        void                    SetGroupBy( sal_Bool bGb=sal_False ){ m_bGroupBy = bGb;}
        void                    SetTabWindow( Window* pWin ){ m_pTabWindow = pWin;}
        void                    SetField( const ::rtl::OUString& rF ){ m_aFieldName = rF;}
        void                    SetFieldAlias( const ::rtl::OUString& rF ){ m_aFieldAlias = rF;}
        void                    SetTable( const ::rtl::OUString& rT ){ m_aTableName = rT;}
        void                    SetAlias( const ::rtl::OUString& rT ){ m_aAliasName = rT;}
        void                    SetFunction( const ::rtl::OUString& rT ) { m_aFunctionName = rT;}
        void                    SetOrderDir( EOrderDir eDir )   { m_eOrderDir = eDir; }
        void                    SetDataType( sal_Int32 eTyp )   { m_eDataType = eTyp; }
        void                    SetFieldType( ETableFieldType eTyp )    { m_eFieldType = eTyp; }
        void                    SetCriteria( sal_uInt16 nIdx, const ::rtl::OUString& rCrit);
        void                    SetColWidth( sal_Int32 nWidth ) { m_nColWidth = nWidth; }
        void                    SetFieldIndex( sal_Int32 nFieldIndex ) { m_nIndex = nFieldIndex; }
        void                    SetFunctionType( sal_Int32 eTyp )   { m_eFunctionType = eTyp; }
        void                    SetColumnId(sal_uInt16 _nColumnId) { m_nColumnId = _nColumnId; }


        ::rtl::OUString         GetField()          const { return m_aFieldName;}
        ::rtl::OUString         GetFieldAlias()     const { return m_aFieldAlias;}
        ::rtl::OUString         GetTable()          const { return m_aTableName;}
        ::rtl::OUString         GetAlias()          const { return m_aAliasName;}
        ::rtl::OUString         GetFunction()       const { return m_aFunctionName;}
        sal_Int32               GetDataType()       const { return m_eDataType; }
        ETableFieldType         GetFieldType()      const { return m_eFieldType; }
        EOrderDir               GetOrderDir()       const { return m_eOrderDir; }
        ::rtl::OUString         GetCriteria( sal_uInt16 nIdx ) const;
        sal_Int32               GetColWidth()       const { return m_nColWidth; }
        sal_Int32               GetFieldIndex()     const { return m_nIndex; }
        Window*                 GetTabWindow()      const { return m_pTabWindow;}
        sal_Int32               GetFunctionType()   const { return m_eFunctionType; }
        sal_uInt16              GetColumnId()       const { return m_nColumnId;}

        inline sal_Bool         isAggreateFunction()    const { return (m_eFunctionType & FKT_AGGREGATE) == FKT_AGGREGATE;  }
        inline sal_Bool         isOtherFunction()       const { return (m_eFunctionType & FKT_OTHER)     == FKT_OTHER;      }
        inline sal_Bool         isNumeric()             const { return (m_eFunctionType & FKT_NUMERIC)   == FKT_NUMERIC;    }
        inline sal_Bool         isNoneFunction()        const { return  m_eFunctionType                  == FKT_NONE;       }
        inline sal_Bool         isCondition()           const { return (m_eFunctionType & FKT_CONDITION) == FKT_CONDITION;  }
        inline sal_Bool         isNumericOrAggreateFunction()   const { return isNumeric() || isAggreateFunction(); }

        sal_Bool                HasCriteria()       const
        {
            ::std::vector< ::rtl::OUString>::const_iterator aIter = m_aCriteria.begin();
            ::std::vector< ::rtl::OUString>::const_iterator aEnd = m_aCriteria.end();
            for(;aIter != aEnd;++aIter)
                if(aIter->getLength())
                    break;
            return aIter != aEnd;
        }

        const ::std::vector< ::rtl::OUString>&  GetCriteria() const { return m_aCriteria; }

        void Load( const ::com::sun::star::beans::PropertyValue& i_rSettings, const bool i_bIncludingCriteria );
        void Save( ::comphelper::NamedValueCollection& o_rSettings, const bool i_bIncludingCriteria );
    };

    //------------------------------------------------------------------
    inline sal_Bool OTableFieldDesc::IsEmpty()  const
    {
        sal_Bool bEmpty = (!m_aTableName.getLength()     &&
                         !m_aAliasName.getLength()       &&
                         !m_aFieldName.getLength()       &&
                         !m_aFieldAlias.getLength()      &&
                         !m_aFunctionName.getLength()    &&
                         !HasCriteria());
        return bEmpty;
    }
    //------------------------------------------------------------------
    typedef ::rtl::Reference< OTableFieldDesc>      OTableFieldDescRef;
    typedef ::std::vector<OTableFieldDescRef>   OTableFields;
}
#endif //

