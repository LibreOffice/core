/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#define _CONNECTIVITY_FILE_VALUE_HXX_

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <comphelper/stl_types.hxx>
#include <rtl/ref.hxx>
#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/CommonTools.hxx"
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdb/XColumn.hpp>

namespace connectivity
{
    namespace detail
    {
        class IValueSource;
    }

    class OOO_DLLPUBLIC_DBTOOLS ORowSetValue
    {
        union
        {
            sal_Bool        m_bBool;
            sal_Int8        m_nInt8;
            sal_Int16       m_nInt16;
            sal_Int32       m_nInt32;
            rtl_uString*    m_pString;

            void*           m_pValue;           // can contains double, etc
        } m_aValue;

        sal_Int32           m_eTypeKind;        // the database type
        sal_Bool            m_bNull     : 1;    // value is null
        sal_Bool            m_bBound    : 1;    // is bound
        sal_Bool            m_bModified : 1;    // value was changed
        sal_Bool            m_bSigned   : 1;    // value is signed

        void free();

    public:
        ORowSetValue()
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
        }

        ORowSetValue(const ORowSetValue& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::rtl::OUString& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const double& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::DOUBLE)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const float& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::FLOAT)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const sal_Int8& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TINYINT)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int16& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::SMALLINT)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int32& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::INTEGER)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int64& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::BIGINT)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const sal_Bool& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::BIT)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::Date& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::DATE)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::Time& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TIME)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::DateTime& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TIMESTAMP)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::uno::Sequence<sal_Int8>& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::LONGVARBINARY)
            ,m_bNull(sal_True)
            ,m_bBound(sal_True)
            ,m_bModified(sal_False)
            ,m_bSigned(sal_True)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ~ORowSetValue()
        {
            free();
        }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }

        ORowSetValue& operator=(const ORowSetValue& _rRH);

        // simple types
        ORowSetValue& operator=(const sal_Bool _rRH);
        ORowSetValue& operator=(const sal_Int8& _rRH);
        ORowSetValue& operator=(const sal_Int16& _rRH);
        ORowSetValue& operator=(const sal_Int32& _rRH);
        ORowSetValue& operator=(const sal_Int64& _rRH);
        ORowSetValue& operator=(const double& _rRH);
        ORowSetValue& operator=(const float& _rRH);

        // ADT's
        ORowSetValue& operator=(const ::com::sun::star::util::Date& _rRH);
        ORowSetValue& operator=(const ::com::sun::star::util::Time& _rRH);
        ORowSetValue& operator=(const ::com::sun::star::util::DateTime& _rRH);

        ORowSetValue& operator=(const ::rtl::OUString& _rRH);
        // the type isn't set it will be set to VARCHAR if the type is different change it
        ORowSetValue& operator=(const ::com::sun::star::uno::Sequence<sal_Int8>& _rRH);
        // we the possiblity to save a any for bookmarks
        ORowSetValue& operator=(const ::com::sun::star::uno::Any& _rAny);

        operator sal_Bool() const   {   return isNull() ? sal_False : getBool();    }
        operator sal_Int8() const   {   return isNull() ? static_cast<sal_Int8>(0) : getInt8(); }
        operator sal_Int16() const  {   return isNull() ? static_cast<sal_Int16>(0) : getInt16();   }
        operator sal_Int32() const  {   return isNull() ? 0         : getInt32();   }
        operator sal_Int64() const  {   return isNull() ? 0         : getLong();    }
        operator float() const      {   return isNull() ? (float)0.0: getFloat();   }
        operator double() const     {   return isNull() ? 0.0       : getDouble();  }

        operator ::rtl::OUString() const
        {
            return isNull() ? ::rtl::OUString() : getString();
        }

        operator ::com::sun::star::util::Date() const
        {
            return isNull() ? ::com::sun::star::util::Date() : getDate();
        }

        operator ::com::sun::star::util::Time() const
        {
            return isNull() ? ::com::sun::star::util::Time() : getTime();
        }

        operator ::com::sun::star::util::DateTime() const
        {
            return isNull() ? ::com::sun::star::util::DateTime() : getDateTime();
        }

        operator ::com::sun::star::uno::Sequence<sal_Int8>() const
        {
            return isNull() ? ::com::sun::star::uno::Sequence<sal_Int8>() : getSequence();
        }

        bool operator==(const ORowSetValue& _rRH) const;
        bool operator!=(const ORowSetValue& _rRH) const
        {
            return !( *this == _rRH );
        }

        sal_Bool    isNull() const
        {
            return m_bNull;
        }
        void        setNull()
        {
            free();
            m_bNull = sal_True;
            m_aValue.m_pString = NULL;
        }

        sal_Bool    isBound() const                     { return m_bBound;      }
        void        setBound(sal_Bool _bBound)          { m_bBound = _bBound ? 1 : 0; }

        sal_Bool    isModified() const                  { return m_bModified;   }
        void        setModified(sal_Bool _bMod=sal_True){ m_bModified = _bMod ? 1 : 0;  }

        sal_Bool    isSigned() const                    { return m_bSigned; }
        void        setSigned(sal_Bool _bMod=sal_True);

        sal_Int32   getTypeKind() const                 { return m_eTypeKind;   }
        void        setTypeKind(sal_Int32 _eType);

        // before calling one of this methods, be sure that the value is not null
        void*           getValue()  const               { OSL_ENSURE(m_bBound,"Value is not bound!");return m_aValue.m_pValue;              }
        sal_Bool        getBool()   const;
        sal_Int8        getInt8()   const;
        sal_Int16       getInt16()  const;
        sal_Int32       getInt32()  const;
        sal_Int64       getLong()   const;
        double          getDouble() const;
        float           getFloat() const;
        // convert the double to the type _nDataType
        void            setFromDouble(const double& _rVal,sal_Int32 _nDatatype);

        ::rtl::OUString getString() const;      // makes a automatic conversion if type isn't a string
        ::com::sun::star::util::Date                getDate()       const;
        ::com::sun::star::util::Time                getTime()       const;
        ::com::sun::star::util::DateTime            getDateTime()   const;
        ::com::sun::star::uno::Sequence<sal_Int8>   getSequence()   const;
        // only use for anys
        ::com::sun::star::uno::Any                  getAny()        const { return *(::com::sun::star::uno::Any*)m_aValue.m_pValue; }
        ::com::sun::star::uno::Any                  makeAny()       const;

        /**
            fetches a single value out of the row
            @param _nPos    the current column position
            @param _nType   the type of the current column
            @param _xRow    the row where to fetch the data from
        */
        void fill(sal_Int32 _nPos,
                  sal_Int32 _nType,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>& _xRow);

        /**
            fetches a single value out of the row
            @param _nPos    the current column position
            @param _nType   the type of the current column
            @param _bNullable   if true then it will be checked if the result could be NULL, otherwise not.
            @param _xRow    the row where to fetch the data from
        */
        void fill(sal_Int32 _nPos,
                  sal_Int32 _nType,
                  sal_Bool  _bNullable,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>& _xRow);

        void fill(const ::com::sun::star::uno::Any& _rValue);

        void fill( const sal_Int32 _nType,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn );

    private:
        void impl_fill( const sal_Int32 _nType, sal_Bool _bNullable, const detail::IValueSource& _rValueSource );
    };

    /// ORowSetValueDecorator decorates a ORowSetValue so the value is "refcounted"
    class OOO_DLLPUBLIC_DBTOOLS ORowSetValueDecorator : public ::salhelper::SimpleReferenceObject
    {
        ORowSetValue    m_aValue;   // my own value
    public:
        ORowSetValueDecorator(){m_aValue.setBound(sal_True);}
        ORowSetValueDecorator(const ORowSetValue& _aValue) : m_aValue(_aValue){m_aValue.setBound(sal_True);}
        ORowSetValueDecorator& operator=(const ORowSetValue& _aValue);

        inline operator const ORowSetValue&()   const               { return m_aValue; }
        inline bool operator ==( const ORowSetValue & _rRH )        { return m_aValue == _rRH; }
        inline const ORowSetValue& getValue()   const               { return m_aValue; }
        inline ORowSetValue& get()                                  { return m_aValue; }
        inline void setValue(const ORowSetValue& _aValue)           { m_aValue = _aValue; }
        inline void setNull()                                       { m_aValue.setNull(); }
        inline void setBound(sal_Bool _bBound )                     { m_aValue.setBound(_bBound);}
        inline sal_Bool isBound( ) const                            { return m_aValue.isBound();}
        inline void setTypeKind(sal_Int32 _nType)                   { m_aValue.setTypeKind(_nType); }
        inline void setModified(sal_Bool _bModified)                { m_aValue.setModified(_bModified); }

    };
    typedef ::rtl::Reference<ORowSetValueDecorator> ORowSetValueDecoratorRef;

    // -------------------------------------------------------------------------
    /// TSetBound is a unary_function to set the bound value with e.q. for_each call
    struct OOO_DLLPUBLIC_DBTOOLS TSetBound : ::std::unary_function<ORowSetValue,void>
    {
        sal_Bool m_bBound;
        TSetBound(sal_Bool _bBound) : m_bBound(_bBound){}
        void operator()(ORowSetValue& _rValue) const { _rValue.setBound(m_bBound); }

    };

    // -------------------------------------------------------------------------
    /// TSetBound is a unary_function to set the bound value with e.q. for_each call
    struct OOO_DLLPUBLIC_DBTOOLS TSetRefBound : ::std::unary_function<ORowSetValueDecoratorRef,void>
    {
        sal_Bool m_bBound;
        TSetRefBound(sal_Bool _bBound) : m_bBound(_bBound){}
        void operator()(ORowSetValueDecoratorRef& _rValue) const { _rValue->setBound(m_bBound); }

    };

    // ----------------------------------------------------------------------------
    // Vector for file based rows
    // ----------------------------------------------------------------------------
    template< class VectorVal > class  ODeleteVector : public connectivity::ORowVector< VectorVal >
    {
        sal_Bool    m_bDeleted;
    public:
        ODeleteVector()             : connectivity::ORowVector< VectorVal >()       ,m_bDeleted(sal_False)  {}
        ODeleteVector(size_t _st)   : connectivity::ORowVector< VectorVal >(_st)    ,m_bDeleted(sal_False)  {}

        sal_Bool    isDeleted() const               { return m_bDeleted;        }
        void        setDeleted(sal_Bool _bDeleted)  { m_bDeleted = _bDeleted;   }
    };

    typedef ODeleteVector< ORowSetValue >               OValueVector;

    class OOO_DLLPUBLIC_DBTOOLS OValueRefVector : public ODeleteVector< ORowSetValueDecoratorRef >
    {
    public:
        OValueRefVector(){}
        OValueRefVector(size_t _st) : ODeleteVector< ORowSetValueDecoratorRef >(_st)
        {
            for(OValueRefVector::Vector::iterator aIter = get().begin() ; aIter != get().end() ;++aIter)
                *aIter = new ORowSetValueDecorator;
        }
    };

#define SQL_NO_PARAMETER (SAL_MAX_UINT32)
    class OAssignValues : public OValueRefVector
    {
        ::std::vector<sal_Int32> m_nParameterIndexes;
    public:
        OAssignValues() : m_nParameterIndexes(1,SQL_NO_PARAMETER){}
        OAssignValues(Vector::size_type n) : OValueRefVector(n),m_nParameterIndexes(n+1,SQL_NO_PARAMETER){}

        void setParameterIndex(sal_Int32 _nId,sal_Int32 _nParameterIndex) { m_nParameterIndexes[_nId] = _nParameterIndex;}
        sal_Int32 getParameterIndex(sal_Int32 _nId) const { return m_nParameterIndexes[_nId]; }
    };

    typedef ::rtl::Reference< OAssignValues > ORefAssignValues;



    typedef ::rtl::Reference< OValueVector >                    OValueRow;
    typedef ::rtl::Reference< OValueRefVector >             OValueRefRow;
}

#endif // #ifndef _CONNECTIVITY_FILE_VALUE_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
