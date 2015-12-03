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

#ifndef INCLUDED_SVX_SMARTTAGMGR_HXX
#define INCLUDED_SVX_SMARTTAGMGR_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <svx/svxdllapi.h>

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
    class XTextRange;
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
     consists of a reference to the service and index.
 */
struct ActionReference
{
    css::uno::Reference< css::smarttags::XSmartTagAction > mxSmartTagAction;
    sal_Int32 mnSmartTagIndex;
    ActionReference( css::uno::Reference< css::smarttags::XSmartTagAction > xSmartTagAction, sal_Int32 nSmartTagIndex )
        : mxSmartTagAction( xSmartTagAction), mnSmartTagIndex( nSmartTagIndex ) {}
};

/** The smart tag manager maintains all installed action and recognizer services

    This class organizes the available smarttag libraries and provides access functions
    to these libraries. The smart tag manager is a singleton.
*/
class SVX_DLLPUBLIC SmartTagMgr : public cppu::WeakImplHelper2< css::util::XModifyListener,
                                                                css::util::XChangesListener >
{
private:

    const OUString maApplicationName;
    std::vector< css::uno::Reference< css::smarttags::XSmartTagRecognizer > > maRecognizerList;
    std::vector< css::uno::Reference< css::smarttags::XSmartTagAction > > maActionList;
    std::set< OUString > maDisabledSmartTagTypes;
    std::multimap < OUString, ActionReference > maSmartTagMap;
    mutable css::uno::Reference< css::i18n::XBreakIterator > mxBreakIter;
    css::uno::Reference< css::uno::XComponentContext> mxContext;
    css::uno::Reference< css::beans::XPropertySet > mxConfigurationSettings;
    bool mbLabelTextWithSmartTags;

    /** Checks for installed smart tag recognizers/actions and stores them in
        maRecognizerList and maActionList.
    */
    void LoadLibraries();

    /** Prepare configuration access.
    */
    void PrepareConfiguration( const OUString& rConfigurationGroupName );

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

    SmartTagMgr( const OUString& rApplicationName );
    virtual ~SmartTagMgr();

    /** Triggeres configuration reading, library loading and listener registration
        NOTE: MUST BE CALLED AFTER CONSTRUCTION!
    */
    void Init( const OUString& rConfigurationGroupName );

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

    void RecognizeString( const OUString& rText,
                    const css::uno::Reference< css::text::XTextMarkup >& rMarkup,
                    const css::uno::Reference< css::frame::XController >& rController,
                    const css::lang::Locale& rLocale,
                    sal_uInt32 nStart, sal_uInt32 nLen ) const;

    void RecognizeTextRange(const css::uno::Reference< css::text::XTextRange>& rRange,
                    const css::uno::Reference< css::text::XTextMarkup >& rMarkup,
                    const css::uno::Reference< css::frame::XController >& rController) const;

    /** Returns all action references associated with a given list of smart tag types

        @param rSmartTagTypes
            The list of types

        @param rActionComponentsSequence
            Output parameter

        @param rActionIndicesSequence
            Output parameter
    */
    void GetActionSequences( std::vector< OUString >& rSmartTagTypes,
                             css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                             css::uno::Sequence < css::uno::Sequence< sal_Int32 > >& rActionIndicesSequence ) const;

    /** Returns the caption for a smart tag type.

        @param rSmartTagType
            The given smart tag type.

        @param rLocale
            The locale.
    */
    OUString GetSmartTagCaption( const OUString& rSmartTagType, const css::lang::Locale& rLocale ) const;

    /** Returns true if the given smart tag type is enabled.
    */
    bool IsSmartTagTypeEnabled( const OUString& rSmartTagType ) const;

    /** Enable or disable smart tags.
    */
    bool IsLabelTextWithSmartTags() const { return mbLabelTextWithSmartTags; }

    /** Returns the number of registered recognizers.
    */
    sal_uInt32 NumberOfRecognizers() const { return maRecognizerList.size(); }

    /** Returns a recognizer.
    */
    css::uno::Reference< css::smarttags::XSmartTagRecognizer >
        GetRecognizer( sal_uInt32 i ) const  { return maRecognizerList[i]; }

    /** Is smart tag recognition active?
    */
    bool IsSmartTagsEnabled() const { return 0 != NumberOfRecognizers() &&
                                      IsLabelTextWithSmartTags(); }

    /** Writes configuration settings.
    */
    void WriteConfiguration( const bool* bLabelTextWithSmartTags,
                             const std::vector< OUString >* pDisabledTypes ) const;

    /** Returns the name of the application this instance has been created by.
    */
    const OUString GetApplicationName() const { return maApplicationName; }

    // css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // css::util::XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::util::XChangesListener
      virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
