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
    class XMultiServiceFactory;
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
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
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
