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


#ifndef _SFX_FCONTNR_HXX
#define _SFX_FCONTNR_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/poolitem.hxx>
#include <vcl/window.hxx>

#include <sfx2/docfilt.hxx>
#include <sfx2/sfxdefs.hxx>

class Window;
class SfxFilter;
class SfxFilterArr_Impl;
class SfxObjectFacArr_Impl;
class SfxObjectFactory;
class SfxMedium;
class SfxFilterContainer_Impl;
class SfxFrame;

//#define SFX_FILTER_CONTAINER_FACTORY 1
typedef sal_uInt16 SfxFilterContainerFlags;

class SfxRefItem : public SfxPoolItem
{
    SvRefBaseRef aRef;
public:
    virtual SfxPoolItem*     Clone( SfxItemPool* = 0 ) const
    {   return new SfxRefItem( *this ); }
    virtual int              operator==( const SfxPoolItem& rL) const
    {   return ((SfxRefItem&)rL).aRef == aRef; }
    SfxRefItem( sal_uInt16 nWhichId, const SvRefBaseRef& rValue ) : SfxPoolItem( nWhichId )
    {   aRef = rValue; }
    const SvRefBaseRef&      GetValue() const { return aRef; }

};

class SfxFrameWindow
{
    Window*     pWindow;
public:
                SfxFrameWindow( Window *pWin )
                 : pWindow( pWin )
                {}

    virtual     ~SfxFrameWindow()
                { delete pWindow; }
    Window*     GetWindow() const
                { return pWindow; }
    void        SetWindow( Window *pWin )
                { pWindow = pWin; }
};

class SfxFrameWindowFactory
{
    typedef SfxFrameWindow* (*FactoryFunc)( SfxFrame* pParent, const String& rName );
    FactoryFunc pFunc;
    String aName;
public:
    SfxFrameWindowFactory( FactoryFunc pFuncP, String aNameP );
    String GetURLWildCard() { return aName; }
    FactoryFunc GetFactory() { return pFunc; }
};

typedef sal_uIntPtr (*SfxDetectFilter)( SfxMedium& rMedium, const SfxFilter **, SfxFilterFlags nMust, SfxFilterFlags nDont );

class SFX2_DLLPUBLIC SfxFilterContainer
{
    SfxFilterContainer_Impl *pImpl;

public:
                        SfxFilterContainer( const String& rName );
                        ~SfxFilterContainer();

//                      SfxFilterContainerFlags GetFlags() const;
//  void                SetFlags( SfxFilterContainerFlags eFlags );

    const String        GetName() const;

    const SfxFilter*    GetAnyFilter( SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4Mime( const String& rMime, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4ClipBoardId( sal_uInt32 nId, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4EA( const String& rEA, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4Extension( const String& rExt, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4FilterName( const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4UIName( const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE static void ReadFilters_Impl( sal_Bool bUpdate=sal_False );
    SAL_DLLPRIVATE static void ReadSingleFilter_Impl( const ::rtl::OUString& rName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xTypeCFG,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xFilterCFG,
                            sal_Bool bUpdate );
    SAL_DLLPRIVATE static const SfxFilter* GetDefaultFilter_Impl( const String& );
//#endif
};

class SfxFilterMatcher_Impl;

class SFX2_DLLPUBLIC SfxFilterMatcher
{
    friend class SfxFilterMatcherIter;
    SfxFilterMatcher_Impl *pImpl;
public:
                        SfxFilterMatcher( const String& rFact );
                        SfxFilterMatcher();
                        ~SfxFilterMatcher();

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE static sal_Bool IsFilterInstalled_Impl( const SfxFilter* pFilter );
    DECL_DLLPRIVATE_STATIC_LINK( SfxFilterMatcher, MaybeFileHdl_Impl, String* );
//#endif

    sal_uInt32               GuessFilterIgnoringContent( SfxMedium& rMedium, const SfxFilter **, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    sal_uInt32               GuessFilter( SfxMedium& rMedium, const SfxFilter **, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    sal_uInt32               GuessFilterControlDefaultUI( SfxMedium& rMedium, const SfxFilter **, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED, sal_Bool bDefUI = sal_True ) const;
    sal_uInt32               DetectFilter( SfxMedium& rMedium, const SfxFilter **, sal_Bool bPlugIn, sal_Bool bAPI = sal_False ) const;

    const SfxFilter*    GetFilter4Mime( const String& rMime, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED) const;
    const SfxFilter*    GetFilter4ClipBoardId( sal_uInt32 nId, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4EA( const String& rEA, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4Extension( const String& rExt, SfxFilterFlags nMust = SFX_FILTER_IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4FilterName( const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilter4UIName( const String& rName, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetFilterForProps( const com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue >& aSeq, SfxFilterFlags nMust = 0, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    const SfxFilter*    GetAnyFilter( SfxFilterFlags nMust=0, SfxFilterFlags nDont=SFX_FILTER_NOTINSTALLED ) const;
};

class SfxFilterContainer_Impl;
class SFX2_DLLPUBLIC SfxFilterMatcherIter
{
    SfxFilterFlags nOrMask;
    SfxFilterFlags nAndMask;
    sal_uInt16 nCurrent;
    const SfxFilterMatcher_Impl *pMatch;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE const SfxFilter* Find_Impl();
//#endif

public:
    SfxFilterMatcherIter( const SfxFilterMatcher* pMatchP, SfxFilterFlags nMask = 0, SfxFilterFlags nNotMask = SFX_FILTER_NOTINSTALLED );
    const SfxFilter* First();
    const SfxFilter* Next();
};


#endif
