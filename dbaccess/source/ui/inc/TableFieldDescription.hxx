/*************************************************************************
 *
 *  $RCSfile: TableFieldDescription.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:54:03 $
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
#ifndef DBAUI_TABLEFIELDDESC_HXX
#define DBAUI_TABLEFIELDDESC_HXX

#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

class Window;
namespace dbaui
{
    class OTableFieldDesc : public ::vos::OReference
    {
    private:
        ::std::vector< ::rtl::OUString> m_vecCriteria;

        ::rtl::OUString             m_aTableName;
        ::rtl::OUString             m_aAliasName;       // table range
        ::rtl::OUString             m_aFieldName;       // column
        ::rtl::OUString             m_aFieldAlias;  // column alias
        ::rtl::OUString             m_aDatabaseName;    // qualifier or catalog
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

        sal_Bool                operator==( const OTableFieldDesc& rDesc );

        void                    clear();

        sal_Bool                IsVisible() const    { return m_bVisible;}
        sal_Bool                IsNumericDataType() const;
        sal_Bool                IsGroupBy() const    { return m_bGroupBy;}

        void                    SetVisible( sal_Bool bVis=sal_True ){ m_bVisible = bVis;}
        void                    SetGroupBy( sal_Bool bGb=sal_False ){ m_bGroupBy = bGb;}
        void                    SetTabWindow( Window* pWin ){ m_pTabWindow = pWin;}
        void                    SetField( const ::rtl::OUString& rF ){ m_aFieldName = rF;}
        void                    SetFieldAlias( const ::rtl::OUString& rF ){ m_aFieldAlias = rF;}
        void                    SetTable( const ::rtl::OUString& rT ){ m_aTableName = rT;}
        void                    SetAlias( const ::rtl::OUString& rT ){ m_aAliasName = rT;}
        void                    SetDatabase( const ::rtl::OUString& rT ) { m_aDatabaseName = rT;}
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
        ::rtl::OUString         GetDatabase()       const { return m_aDatabaseName;}
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
            ::std::vector< ::rtl::OUString>::const_iterator aIter = m_vecCriteria.begin();
            for(;aIter != m_vecCriteria.end();++aIter)
                if(aIter->getLength())
                    break;
            return aIter != m_vecCriteria.end();
        }

        void                    NextOrderDir();
        const ::std::vector< ::rtl::OUString>&  GetCriteria() const { return m_vecCriteria;}

        void Load(const ::com::sun::star::beans::PropertyValue& _rProperty);
        void Save(::com::sun::star::beans::PropertyValue& _rProperty);
    };

    //------------------------------------------------------------------
    inline sal_Bool OTableFieldDesc::IsEmpty()  const
    {
        sal_Bool bEmpty = (!m_aTableName.getLength()     &&
                         !m_aAliasName.getLength()       &&
                         !m_aFieldName.getLength()       &&
                         !m_aFieldAlias.getLength()      &&
                         !m_aDatabaseName.getLength()    &&
                         !m_aFunctionName.getLength()    &&
                         !HasCriteria());
        return bEmpty;
    }
    //------------------------------------------------------------------
    typedef ::vos::ORef< OTableFieldDesc>       OTableFieldDescRef;
    typedef ::std::vector<OTableFieldDescRef>   OTableFields;
}
#endif //

