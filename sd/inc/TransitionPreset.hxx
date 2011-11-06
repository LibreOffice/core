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
 *************************************************************/



#ifndef _SD_TRANSITIONPRESET_HXX
#define _SD_TRANSITIONPRESET_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _UTL_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include <list>
#include <hash_map>

namespace com { namespace sun { namespace star {
    namespace animations { class XAnimationNode; }
    namespace uno { template<class X> class Reference; }
} } }

class SdPage;
class String;

namespace sd {

class TransitionPreset;
typedef boost::shared_ptr< TransitionPreset > TransitionPresetPtr;
typedef std::list< TransitionPresetPtr > TransitionPresetList;
typedef std::hash_map< rtl::OUString, rtl::OUString, comphelper::UStringHash, comphelper::UStringEqual > UStringMap;

class TransitionPreset
{
public:
    static const TransitionPresetList& getTransitionPresetList();
    static bool importTransitionPresetList( TransitionPresetList& rList );

    void apply( SdPage* pSlide ) const;

    sal_Int16 getTransition() const { return mnTransition; }
    sal_Int16 getSubtype() const { return mnSubtype; }
    sal_Bool getDirection() const { return mbDirection; }
    sal_Int32 getFadeColor() const { return mnFadeColor; }

    const rtl::OUString& getUIName() const { return maUIName; }
    const rtl::OUString& getPresetId() const { return maPresetId; }

private:
    TransitionPreset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    sal_Int16 mnTransition;
    sal_Int16 mnSubtype;
    sal_Bool mbDirection;
    sal_Int32 mnFadeColor;
    rtl::OUString maPresetId;
    rtl::OUString maUIName;

    static sd::TransitionPresetList* mpTransitionPresetList;

    static bool importTransitionsFile( TransitionPresetList& rList,
                                       ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                                       UStringMap& rTransitionNameMape,
                                       String aFilaname );
};

}

#endif // _SD_TRANSITIONPRESET_HXX

