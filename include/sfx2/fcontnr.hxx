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
#ifndef INCLUDED_SFX2_FCONTNR_HXX
#define INCLUDED_SFX2_FCONTNR_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/poolitem.hxx>
#include <vcl/window.hxx>

#include <sfx2/docfilt.hxx>

#include <tools/ref.hxx>
#include <memory>

namespace vcl { class Window; }
class SfxFilter;
class SfxObjectFactory;
class SfxMedium;
class SfxFilterContainer_Impl;
class SfxFrame;


class SFX2_DLLPUBLIC SfxFilterContainer
{
    std::unique_ptr<SfxFilterContainer_Impl> pImpl;

public:
                        SfxFilterContainer( const OUString& rName );
                        ~SfxFilterContainer();


    const OUString      GetName() const;

    std::shared_ptr<const SfxFilter>    GetAnyFilter( SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4EA( const OUString& rEA, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4Extension( const OUString& rExt, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4FilterName( const OUString& rName, SfxFilterFlags nMust = SfxFilterFlags::NONE, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;

    SAL_DLLPRIVATE static void ReadFilters_Impl( bool bUpdate=false );
    SAL_DLLPRIVATE static void ReadSingleFilter_Impl( const OUString& rName,
                            const css::uno::Reference< css::container::XNameAccess >& xTypeCFG,
                            const css::uno::Reference< css::container::XNameAccess >& xFilterCFG,
                            bool bUpdate );
    SAL_DLLPRIVATE static std::shared_ptr<const SfxFilter> GetDefaultFilter_Impl( const OUString& );
};

class SfxFilterMatcher_Impl;

class SFX2_DLLPUBLIC SfxFilterMatcher
{
    friend class SfxFilterMatcherIter;
    SfxFilterMatcher_Impl &m_rImpl;
public:
                        SfxFilterMatcher( const OUString& rFact );
                        SfxFilterMatcher();
                        ~SfxFilterMatcher();
                        SfxFilterMatcher(const SfxFilterMatcher&) = delete;
    SfxFilterMatcher&   operator=( const SfxFilterMatcher& ) = delete;

    SAL_DLLPRIVATE static bool IsFilterInstalled_Impl( const std::shared_ptr<const SfxFilter>& pFilter );
    DECL_DLLPRIVATE_LINK( MaybeFileHdl_Impl, OUString*, bool );

    ErrCode                  GuessFilterIgnoringContent( SfxMedium const & rMedium, std::shared_ptr<const SfxFilter>& ) const;
    ErrCode                  GuessFilter( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>& , SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    ErrCode                  GuessFilterControlDefaultUI( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>&, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    ErrCode                  DetectFilter( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>& ) const;

    std::shared_ptr<const SfxFilter>    GetFilter4Mime( const OUString& rMime, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED) const;
    std::shared_ptr<const SfxFilter>    GetFilter4ClipBoardId( SotClipboardFormatId nId, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4EA( const OUString& rEA, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4Extension( const OUString& rExt, SfxFilterFlags nMust = SfxFilterFlags::IMPORT, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4FilterName( const OUString& rName, SfxFilterFlags nMust = SfxFilterFlags::NONE, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilter4UIName( const OUString& rName, SfxFilterFlags nMust = SfxFilterFlags::NONE, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetFilterForProps( const css::uno::Sequence < css::beans::NamedValue >& aSeq, SfxFilterFlags nMust = SfxFilterFlags::NONE, SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED ) const;
    std::shared_ptr<const SfxFilter>    GetAnyFilter( SfxFilterFlags nMustg=SfxFilterFlags::NONE, SfxFilterFlags nDont=SFX_FILTER_NOTINSTALLED ) const;
};

class SfxFilterContainer_Impl;
class SFX2_DLLPUBLIC SfxFilterMatcherIter

{
    SfxFilterFlags nOrMask;
    SfxFilterFlags nAndMask;
    sal_uInt16 nCurrent;
    const SfxFilterMatcher_Impl &m_rMatch;

    SAL_DLLPRIVATE std::shared_ptr<const SfxFilter> Find_Impl();

public:
    SfxFilterMatcherIter( const SfxFilterMatcher& rMatcher, SfxFilterFlags nMask = SfxFilterFlags::NONE, SfxFilterFlags nNotMask = SFX_FILTER_NOTINSTALLED );
    SfxFilterMatcherIter(const SfxFilterMatcherIter&) = delete;
    SfxFilterMatcherIter& operator=( const SfxFilterMatcherIter& ) = delete;
    std::shared_ptr<const SfxFilter> First();
    std::shared_ptr<const SfxFilter> Next();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
