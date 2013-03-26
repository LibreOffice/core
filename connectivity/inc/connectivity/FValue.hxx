/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#define _CONNECTIVITY_FILE_VALUE_HXX_

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>
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
            bool            m_bBool;

            sal_Int8        m_nInt8;
            sal_uInt8       m_uInt8;

            sal_Int16       m_nInt16;
            sal_uInt16      m_uInt16;

            sal_Int32       m_nInt32;
            sal_uInt32      m_uInt32;

            sal_Int64       m_nInt64;
            sal_uInt64      m_uInt64;

            float           m_nFloat;
            double          m_nDouble;

            rtl_uString*    m_pString;

            void*           m_pValue;           // date/time/timestamp/sequence
        } m_aValue;

        sal_Int32           m_eTypeKind;        // the database type
        bool                m_bNull     : 1;    // value is null
        bool                m_bBound    : 1;    // is bound
        bool                m_bModified : 1;    // value was changed
        bool                m_bSigned   : 1;    // value is signed

        void free();

    public:
        ORowSetValue()
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
        }

        ORowSetValue(const ORowSetValue& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::rtl::OUString& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::VARCHAR)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const double& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::DOUBLE)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const float& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::FLOAT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const sal_Int8& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TINYINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const sal_uInt8& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TINYINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(false)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int16& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::SMALLINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_uInt16& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::SMALLINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(false)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int32& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::INTEGER)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_uInt32& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::INTEGER)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(false)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_Int64& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::BIGINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }
        ORowSetValue(const sal_uInt64& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::BIGINT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(false)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const bool& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::BIT)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::Date& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::DATE)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::Time& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TIME)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::util::DateTime& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::TIMESTAMP)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ORowSetValue(const ::com::sun::star::uno::Sequence<sal_Int8>& _rRH)
            :m_eTypeKind(::com::sun::star::sdbc::DataType::LONGVARBINARY)
            ,m_bNull(true)
            ,m_bBound(true)
            ,m_bModified(false)
            ,m_bSigned(true)
        {
            m_aValue.m_pString = NULL;
            operator=(_rRH);
        }

        ~ORowSetValue()
        {
            free();
        }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW(())
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW(())
            {  }

        ORowSetValue& operator=(const ORowSetValue& _rRH);

        // simple types
        ORowSetValue& operator=(const bool _rRH);

        ORowSetValue& operator=(const sal_Int8& _rRH);
        ORowSetValue& operator=(const sal_uInt8& _rRH);

        ORowSetValue& operator=(const sal_Int16& _rRH);
        ORowSetValue& operator=(const sal_uInt16& _rRH);

        ORowSetValue& operator=(const sal_Int32& _rRH);
        ORowSetValue& operator=(const sal_uInt32& _rRH);

        ORowSetValue& operator=(const sal_Int64& _rRH);
        ORowSetValue& operator=(const sal_uInt64& _rRH);

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

        operator bool() const   {   return isNull() ? false : getBool();    }
        operator sal_Int8() const   {   return isNull() ? static_cast<sal_Int8>(0) : getInt8();   }
        operator sal_uInt8() const  {   return isNull() ? static_cast<sal_uInt8>(0) : getUInt8(); }

        operator sal_Int16() const  {   return isNull() ? static_cast<sal_Int16>(0) : getInt16();   }
        operator sal_uInt16() const {   return isNull() ? static_cast<sal_uInt16>(0) : getUInt16(); }

        operator sal_Int32() const  {   return isNull() ? 0         : getInt32();   }
        operator sal_uInt32() const {   return isNull() ? 0         : getUInt32();  }

        operator sal_Int64() const  {   return isNull() ? 0         : getLong();    }
        operator sal_uInt64() const {   return isNull() ? 0         : getULong();   }

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

        bool isNull() const
        {
            return m_bNull;
        }
        void        setNull()
        {
            free();
            m_bNull = true;
            m_aValue.m_pString = NULL;
        }

        bool        isBound() const                     { return m_bBound;      }
        void        setBound(bool _bBound)              { m_bBound = _bBound ? 1 : 0; }

        bool        isModified() const                  { return m_bModified;   }
        void        setModified(bool _bMod=true)        { m_bModified = _bMod ? 1 : 0;  }

        bool        isSigned() const                    { return m_bSigned; }
        void        setSigned(bool _bSig=true);

        sal_Int32   getTypeKind() const                 { return m_eTypeKind;   }
        void        setTypeKind(sal_Int32 _eType);

        // before calling one of this methods, be sure that the value is not null
        void*           getValue()  const               { OSL_ENSURE(m_bBound,"Value is not bound!");return m_aValue.m_pValue;              }
        bool            getBool()   const;

        sal_Int8        getInt8()   const;
        sal_uInt8       getUInt8()  const;

        sal_Int16       getInt16()  const;
        sal_uInt16      getUInt16() const;

        sal_Int32       getInt32()  const;
        sal_uInt32      getUInt32() const;

        sal_Int64       getLong()   const;
        sal_uInt64      getULong()  const;

        double          getDouble() const;
        float           getFloat()  const;

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
                  bool      _bNullable,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>& _xRow);

        void fill(const ::com::sun::star::uno::Any& _rValue);

        void fill( const sal_Int32 _nType,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn );

    private:
        void impl_fill( const sal_Int32 _nType, bool _bNullable, const detail::IValueSource& _rValueSource );
    };

    /// ORowSetValueDecorator decorates a ORowSetValue so the value is "refcounted"
    class OOO_DLLPUBLIC_DBTOOLS ORowSetValueDecorator : public ::salhelper::SimpleReferenceObject
    {
        ORowSetValue    m_aValue;   // my own value
    public:
        ORowSetValueDecorator(){m_aValue.setBound(true);}
        ORowSetValueDecorator(const ORowSetValue& _aValue) : m_aValue(_aValue){m_aValue.setBound(true);}
        ORowSetValueDecorator& operator=(const ORowSetValue& _aValue);

        inline operator const ORowSetValue&()   const               { return m_aValue; }
        inline bool operator ==( const ORowSetValue & _rRH )        { return m_aValue == _rRH; }
        inline const ORowSetValue& getValue()   const               { return m_aValue; }
        inline ORowSetValue& get()                                  { return m_aValue; }
        inline void setValue(const ORowSetValue& _aValue)           { m_aValue = _aValue; }
        inline void setNull()                                       { m_aValue.setNull(); }
        inline void setBound(bool _bBound )                         { m_aValue.setBound(_bBound);}
        inline bool isBound( ) const                                { return m_aValue.isBound();}
        inline void setTypeKind(sal_Int32 _nType)                   { m_aValue.setTypeKind(_nType); }
        inline void setModified(bool _bModified)                    { m_aValue.setModified(_bModified); }

    };
    typedef ::rtl::Reference<ORowSetValueDecorator> ORowSetValueDecoratorRef;

    // -------------------------------------------------------------------------
    /// TSetBound is a unary_function to set the bound value with e.q. for_each call
    struct OOO_DLLPUBLIC_DBTOOLS TSetBound : ::std::unary_function<ORowSetValue,void>
    {
        bool m_bBound;
        TSetBound(bool _bBound) : m_bBound(_bBound){}
        void operator()(ORowSetValue& _rValue) const { _rValue.setBound(m_bBound); }

    };

    // -------------------------------------------------------------------------
    /// TSetBound is a unary_function to set the bound value with e.q. for_each call
    struct OOO_DLLPUBLIC_DBTOOLS TSetRefBound : ::std::unary_function<ORowSetValueDecoratorRef,void>
    {
        bool m_bBound;
        TSetRefBound(bool _bBound) : m_bBound(_bBound){}
        void operator()(ORowSetValueDecoratorRef& _rValue) const { _rValue->setBound(m_bBound); }

    };

    // ----------------------------------------------------------------------------
    // Vector for file based rows
    // ----------------------------------------------------------------------------
    template< class VectorVal > class  ODeleteVector : public connectivity::ORowVector< VectorVal >
    {
        bool    m_bDeleted;
    public:
        ODeleteVector()             : connectivity::ORowVector< VectorVal >()       ,m_bDeleted(false)  {}
        ODeleteVector(size_t _st)   : connectivity::ORowVector< VectorVal >(_st)    ,m_bDeleted(false)  {}

        bool isDeleted() const           { return m_bDeleted;        }
        void setDeleted(bool _bDeleted)  { m_bDeleted = _bDeleted;   }
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
