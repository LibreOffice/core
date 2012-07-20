/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
//This file is about the conversion of the UOF v2.0 and ODF document format from CS2C 20120610.
#ifndef FILTER_SOURCE_XSLTFILTER_CONTAINERHELPER_HXX
#define FILTER_SOURCE_XSLTFILTER_CONTAINERHELPER_HXX

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace XSLT{

template< typename KeyType, typename ObjType, typename CompType = ::std::less< KeyType > >
class RefMap : public ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType >
{
public:
    typedef ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType > container_type;
    typedef typename container_type::key_type       key_type;
    typedef typename container_type::mapped_type    mapped_type;
    typedef typename container_type::value_type     value_type;
    typedef typename container_type::key_compare    key_compare;

    inline bool has( key_type nKey ) const
    {
        const mapped_type* pxRef = getRef(nKey);
        return pxRef && pxRef->get();
    }

    inline mapped_type get( key_type nKey ) const
    {
        if(const mapped_type* pxRef = getRef(nKey) ) return *pxRef;
        return mapped_type();
    }

    template<typename FunctorType >
    inline void forEach( const FunctorType& rFunctor) const
    {
        ::std::for_each(this->begin(), this->end(), ForEachFunctor< FunctorType >(rFunctor));
    }

    template< typename FuncType >
    inline void forEachMem(FuncType pFunc)
    {
        forEach( ::boost::bind(pFunc, _1 ));
    }

    template<typename FuncType, typename ParamType>
    inline void forEachMem(FuncType pFunc, ParamType aParam) const
    {
        forEach( ::boost::bind(pFunc, _1, aParam));
    }

    template<typename FuncType, typename ParamType1, typename ParamType2>
    inline void forEachMem(FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2) const
    {
        forEach( ::boost::bind(pFunc, -1, aParam1, aParam2 ));
    }

    template<typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3>
    inline void forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
    {
        forEach( ::boost::bind(pFunc, _1, aParam2, aParam2, aParam3 ));
    }

    template<typename FuncType>
    inline void forEachWithKey(const FuncType& rFunctor) const
    {
        ::std::for_each( this->begin(), this->end(), ForEachFunctorWithKey< FuncType >(rFunctor));
    }

    template<typename FuncType>
    inline void forEachMemWithKey(FuncType pFunc) const
    {
        forEachWithKey( ::boost::bind(pFunc, _2, _1));
    }

    template<typename FuncType, typename ParamType>
    inline void forEachMemWithKey(FuncType pFunc, ParamType aParam1) const
    {
        forEachWithKey( ::boost::bind(pFunc, _2, _1, aParam1) );
    }

    template<typename FuncType, typename ParamType1, typename ParamType2>
    inline void forEachMemWithKey(FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2) const
    {
        forEachWithKey( ::boost::bind(pFunc, _2, _1, aParam1, aParam2) );
    }

    template<typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3>
    inline void forEachMemWithKey(FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3) const
    {
        forEachWithKey( ::boost::bind(pFunc, _2, _1, aParam1, aParam2, aParam3) );
    }
private:
    template<typename FunctorType>
    struct ForEachFunctor
    {
        FunctorType m_aFunctor;
        inline explicit ForEachFunctor( const FunctorType& rFunctor): m_aFunctor(rFunctor){}
        inline void operator()(const value_type& rValue)
        {
            if(rValue.second.get())
                m_aFunctor(*rValue.second);
        }
    };

    template<typename FunctorType>
    struct ForEachFunctorWithKey
    {
        FunctorType m_aFunctor;
        inline explicit ForEachFunctorWithKey( const FunctorType& rFunctor) : m_aFunctor(rFunctor){}
        inline void operator()(const value_type& rValue)
        {
            if(rValue.second.get())
                m_aFunctor(rValue.first, *rValue.second);
        }
    };

    inline const mapped_type* getRef( key_type nKey ) const
    {
        typename container_type::const_iterator aIt = find(nKey);
        return (aIt == this->end())? 0 : &aIt->second;
    }
};

}

#endif
