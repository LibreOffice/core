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

#ifndef _SMARTTAGMGR_HXX
#define _SMARTTAGMGR_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include "svx/svxdllapi.h"

#include <vector>
#include <map>
#include <set>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace com { namespace sun { namespace star { namespace smarttags {
    class XSmartTagRecognizer;
    class XSmartTagAction;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextMarkup;
} } } }

namespace com { namespace sun { namespace star { namespace i18n {
    class XBreakIterator;
} } } }

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
    struct EventObject;
    struct ChangesEvent;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySet;
} } } }

namespace com { namespace sun { namespace star { namespace frame {
    class XController;
} } } }

/** A reference to a smart tag action

     An action service can support various actions. Therefore an ActionReference
     consists of a reference to the service and and index.
 */
struct ActionReference
{
    com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > mxSmartTagAction;
    sal_Int32 mnSmartTagIndex;
    ActionReference( com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > xSmartTagAction, sal_Int32 nSmartTagIndex )
        : mxSmartTagAction( xSmartTagAction), mnSmartTagIndex( nSmartTagIndex ) {}
};

/** The smart tag manager maintains all installed action and recognizer services

    This class organizes the available smarttag libraries and provides access functions
    to these libraries. The smart tag manager is a singleton.
*/
class SVX_DLLPUBLIC SmartTagMgr : public cppu::WeakImplHelper2< ::com::sun::star::util::XModifyListener,
                                                                ::com::sun::star::util::XChangesListener >
{
private:

    const rtl::OUString maApplicationName;
    std::vector< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagRecognizer > > maRecognizerList;
    std::vector< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > maActionList;
    std::set< rtl::OUString > maDisabledSmartTagTypes;
    std::multimap < rtl::OUString, ActionReference > maSmartTagMap;
    mutable com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > mxBreakIter;
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> mxContext;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > mxConfigurationSettings;
    bool mbLabelTextWithSmartTags;

    /** Checks for installed smart tag recognizers/actions and stores them in
        maRecognizerList and maActionList.
    */
    void LoadLibraries();

    /** Prepare configuration access.
    */
    void PrepareConfiguration( const rtl::OUString& rConfigurationGroupName );

    /** Reads the configuration data.
    */
    void ReadConfiguration( bool bExcludedTypes, bool bRecognize );

    /** Registeres the smart tag manager as listener at the package manager.
    */
    void RegisterListener();

    /** Sets up a map that maps smart tag type names to actions references.
    */
    void AssociateActionsWithRecognizers();

    void CreateBreakIterator() const;

public:

    SmartTagMgr( const rtl::OUString& rApplicationName );
    virtual ~SmartTagMgr();

    /** Triggeres configuration reading, library loading and listener registration
        NOTE: MUST BE CALLED AFTER CONSTRUCTION!
    */
    void Init( const rtl::OUString& rConfigurationGroupName );

    /** Dispatches the recognize call to all installed smart tag recognizers

        @param rText
            The string to be scanned by the recognizers.

        @param xMarkup
            The object allows the recognizers to store any found smart tags.

        @param xController
                The current controller of the document.

        @param rLocale
            The locale of rText.

        @param nStart
            The start offset of the text to be scanned in rText.

        @param nLen
            The length of the text to be scanned.

    */
    void Recognize( const rtl::OUString& rText,
                    const com::sun::star::uno::Reference< com::sun::star::text::XTextMarkup > xMarkup,
                    const com::sun::star::uno::Reference< com::sun::star::frame::XController > xController,
                    const com::sun::star::lang::Locale& rLocale,
                    sal_uInt32 nStart, sal_uInt32 nLen ) const;

    /** Returns all action references associated with a given list of smart tag types

        @param rSmartTagTypes
            The list of types

        @param rActionComponentsSequence
            Output parameter

        @param rActionIndicesSequence
            Output parameter
    */
    void GetActionSequences( com::sun::star::uno::Sequence < rtl::OUString >& rSmartTagTypes,
                             com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                             com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence ) const;

    /** Returns the caption for a smart tag type.

        @param rSmartTagType
            The given smart tag type.

        @param rLocale
            The locale.
    */
    rtl::OUString GetSmartTagCaption( const rtl::OUString& rSmartTagType, const com::sun::star::lang::Locale& rLocale ) const;

    /** Returns true if the given smart tag type is enabled.
    */
    bool IsSmartTagTypeEnabled( const rtl::OUString& rSmartTagType ) const;

    /** Enable or disable smart tags.
    */
    bool IsLabelTextWithSmartTags() const { return mbLabelTextWithSmartTags; }

    /** Returns the number of registered recognizers.
    */
    sal_uInt32 NumberOfRecognizers() const { return maRecognizerList.size(); }

    /** Returns a recognizer.
    */
    com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagRecognizer >
        GetRecognizer( sal_uInt32 i ) const  { return maRecognizerList[i]; }

    /** Is smart tag recognization active?
    */
    bool IsSmartTagsEnabled() const { return 0 != NumberOfRecognizers() &&
                                      IsLabelTextWithSmartTags(); }

    /** Writes configuration settings.
    */
    void WriteConfiguration( const bool* bLabelTextWithSmartTags,
                             const std::vector< rtl::OUString >* pDisabledTypes ) const;

    /** Returns the name of the application this instance has been created by.
    */
    const rtl::OUString GetApplicationName() const { return maApplicationName; }

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XChangesListener
      virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
