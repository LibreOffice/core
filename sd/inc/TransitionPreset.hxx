/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransitionPreset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:33:35 $
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

#ifndef _SD_TRANSITIONPRESET_HXX
#define _SD_TRANSITIONPRESET_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

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

