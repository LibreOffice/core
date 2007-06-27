/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmartTagMgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:37:13 $
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
 *    Initial Contributer was Fabalabs Software GmbH, Jakob Lechner
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

#ifndef _SMARTTAGMGR_HXX
#define _SMARTTAGMGR_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > mxBreakIter;
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
