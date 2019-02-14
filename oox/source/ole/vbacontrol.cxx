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

#include <oox/ole/vbacontrol.hxx>

#include <algorithm>
#include <set>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/storagebase.hxx>
#include <oox/helper/textinputstream.hxx>
#include <oox/ole/vbahelper.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <unordered_map>

namespace oox {
namespace ole {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace {

const sal_uInt16 VBA_SITE_CLASSIDINDEX          = 0x8000;
const sal_uInt16 VBA_SITE_INDEXMASK             = 0x7FFF;
const sal_uInt16 VBA_SITE_FORM                  = 7;
const sal_uInt16 VBA_SITE_IMAGE                 = 12;
const sal_uInt16 VBA_SITE_FRAME                 = 14;
const sal_uInt16 VBA_SITE_SPINBUTTON            = 16;
const sal_uInt16 VBA_SITE_COMMANDBUTTON         = 17;
const sal_uInt16 VBA_SITE_TABSTRIP              = 18;
const sal_uInt16 VBA_SITE_LABEL                 = 21;
const sal_uInt16 VBA_SITE_TEXTBOX               = 23;
const sal_uInt16 VBA_SITE_LISTBOX               = 24;
const sal_uInt16 VBA_SITE_COMBOBOX              = 25;
const sal_uInt16 VBA_SITE_CHECKBOX              = 26;
const sal_uInt16 VBA_SITE_OPTIONBUTTON          = 27;
const sal_uInt16 VBA_SITE_TOGGLEBUTTON          = 28;
const sal_uInt16 VBA_SITE_SCROLLBAR             = 47;
const sal_uInt16 VBA_SITE_MULTIPAGE             = 57;
const sal_uInt16 VBA_SITE_UNKNOWN               = 0x7FFF;

const sal_uInt32 VBA_SITE_TABSTOP               = 0x00000001;
const sal_uInt32 VBA_SITE_VISIBLE               = 0x00000002;
const sal_uInt32 VBA_SITE_OSTREAM               = 0x00000010;
const sal_uInt32 VBA_SITE_DEFFLAGS              = 0x00000033;

const sal_uInt8 VBA_SITEINFO_COUNT              = 0x80;
const sal_uInt8 VBA_SITEINFO_MASK               = 0x7F;

/** Collects names of all controls in a user form or container control. Allows
    to generate unused names for dummy controls separating option groups.
 */
class VbaControlNamesSet
{
public:
    explicit            VbaControlNamesSet();

    /** Inserts the name of the passed control. */
    void                insertName( const VbaFormControl& rControl );
    /** Returns a name that is not contained in this set. */
    OUString            generateDummyName();

private:
    ::std::set< OUString >
                        maCtrlNames;
    sal_Int32           mnIndex;
};

static const OUStringLiteral gaDummyBaseName( "DummyGroupSep" );

VbaControlNamesSet::VbaControlNamesSet() :
    mnIndex( 0 )
{
}

void VbaControlNamesSet::insertName( const VbaFormControl& rControl )
{
    OUString aName = rControl.getControlName();
    if( !aName.isEmpty() )
        maCtrlNames.insert( aName );
}

OUString VbaControlNamesSet::generateDummyName()
{
    OUString aCtrlName;
    do
    {
        aCtrlName = gaDummyBaseName + OUString::number( ++mnIndex );
    }
    while( maCtrlNames.count( aCtrlName ) > 0 );
    maCtrlNames.insert( aCtrlName );
    return aCtrlName;
}

/** Functor that inserts the name of a control into a VbaControlNamesSet. */
struct VbaControlNameInserter
{
public:
    VbaControlNamesSet& mrCtrlNames;
    explicit     VbaControlNameInserter( VbaControlNamesSet& rCtrlNames ) : mrCtrlNames( rCtrlNames ) {}
    void         operator()( const VbaFormControl& rControl ) { mrCtrlNames.insertName( rControl ); }
};

/** A dummy invisible form control (fixed label without text) that is used to
    separate two groups of option buttons.
 */
class VbaDummyFormControl : public VbaFormControl
{
public:
    explicit            VbaDummyFormControl( const OUString& rName );
};

VbaDummyFormControl::VbaDummyFormControl( const OUString& rName )
{
    mxSiteModel.reset( new VbaSiteModel );
    mxSiteModel->importProperty( XML_Name, rName );
    mxSiteModel->importProperty( XML_VariousPropertyBits, OUString( '0' ) );

    mxCtrlModel.reset( new AxLabelModel );
    mxCtrlModel->setAwtModelMode();
    mxCtrlModel->importProperty( XML_Size, "10;10" );
}

} // namespace

VbaSiteModel::VbaSiteModel() :
    maPos( 0, 0 ),
    mnId( 0 ),
    mnHelpContextId( 0 ),
    mnFlags( VBA_SITE_DEFFLAGS ),
    mnStreamLen( 0 ),
    mnTabIndex( -1 ),
    mnClassIdOrCache( VBA_SITE_UNKNOWN ),
    mnGroupId( 0 )
{
}

VbaSiteModel::~VbaSiteModel()
{
}

void VbaSiteModel::importProperty( sal_Int32 nPropId, const OUString& rValue )
{
    switch( nPropId )
    {
        case XML_Name:                  maName = rValue;                                            break;
        case XML_Tag:                   maTag = rValue;                                             break;
        case XML_VariousPropertyBits:   mnFlags = AttributeConversion::decodeUnsigned( rValue );    break;
    }
}

bool VbaSiteModel::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readStringProperty( maName );
    aReader.readStringProperty( maTag );
    aReader.readIntProperty< sal_Int32 >( mnId );
    aReader.readIntProperty< sal_Int32 >( mnHelpContextId );
    aReader.readIntProperty< sal_uInt32 >( mnFlags );
    aReader.readIntProperty< sal_uInt32 >( mnStreamLen );
    aReader.readIntProperty< sal_Int16 >( mnTabIndex );
    aReader.readIntProperty< sal_uInt16 >( mnClassIdOrCache );
    aReader.readPairProperty( maPos );
    aReader.readIntProperty< sal_uInt16 >( mnGroupId );
    aReader.skipUndefinedProperty();
    aReader.readStringProperty( maToolTip );
    aReader.skipStringProperty();   // license key
    aReader.readStringProperty( maControlSource );
    aReader.readStringProperty( maRowSource );
    return aReader.finalizeImport();
}

void VbaSiteModel::moveRelative( const AxPairData& rDistance )
{
    maPos.first += rDistance.first;
    maPos.second += rDistance.second;
}

bool VbaSiteModel::isContainer() const
{
    return !getFlag( mnFlags, VBA_SITE_OSTREAM );
}

sal_uInt32 VbaSiteModel::getStreamLength() const
{
    return isContainer() ? 0 : mnStreamLen;
}

OUString VbaSiteModel::getSubStorageName() const
{
    if( mnId >= 0 )
    {
        OUStringBuffer aBuffer;
        aBuffer.append( 'i' );
        if( mnId < 10 )
            aBuffer.append( '0' );
        aBuffer.append( mnId );
        return aBuffer.makeStringAndClear();
    }
    return OUString();
}

ControlModelRef VbaSiteModel::createControlModel( const AxClassTable& rClassTable ) const
{
    ControlModelRef xCtrlModel;

    sal_Int32 nTypeIndex = static_cast< sal_Int32 >( mnClassIdOrCache & VBA_SITE_INDEXMASK );
    if( !getFlag( mnClassIdOrCache, VBA_SITE_CLASSIDINDEX ) )
    {
        switch( nTypeIndex )
        {
            case VBA_SITE_COMMANDBUTTON:    xCtrlModel.reset( new AxCommandButtonModel );   break;
            case VBA_SITE_LABEL:            xCtrlModel.reset( new AxLabelModel );           break;
            case VBA_SITE_IMAGE:            xCtrlModel.reset( new AxImageModel );           break;
            case VBA_SITE_TOGGLEBUTTON:     xCtrlModel.reset( new AxToggleButtonModel );    break;
            case VBA_SITE_CHECKBOX:         xCtrlModel.reset( new AxCheckBoxModel );        break;
            case VBA_SITE_OPTIONBUTTON:     xCtrlModel.reset( new AxOptionButtonModel );    break;
            case VBA_SITE_TEXTBOX:          xCtrlModel.reset( new AxTextBoxModel );         break;
            case VBA_SITE_LISTBOX:          xCtrlModel.reset( new AxListBoxModel );         break;
            case VBA_SITE_COMBOBOX:         xCtrlModel.reset( new AxComboBoxModel );        break;
            case VBA_SITE_SPINBUTTON:       xCtrlModel.reset( new AxSpinButtonModel );      break;
            case VBA_SITE_SCROLLBAR:        xCtrlModel.reset( new AxScrollBarModel );       break;
            case VBA_SITE_TABSTRIP:         xCtrlModel.reset( new AxTabStripModel );
            break;
            case VBA_SITE_FRAME:            xCtrlModel.reset( new AxFrameModel );           break;
            case VBA_SITE_MULTIPAGE:        xCtrlModel.reset( new AxMultiPageModel );
            break;
            case VBA_SITE_FORM:             xCtrlModel.reset( new AxPageModel );
            break;
            default:    OSL_FAIL( "VbaSiteModel::createControlModel - unknown type index" );
        }
    }
    else
    {
        const OUString* pGuid = ContainerHelper::getVectorElement( rClassTable, nTypeIndex );
        OSL_ENSURE( pGuid, "VbaSiteModel::createControlModel - invalid class table index" );
        if( pGuid )
        {
            if( *pGuid == COMCTL_GUID_SCROLLBAR_60 )
                xCtrlModel.reset( new ComCtlScrollBarModel( 6 ) );
            else if( *pGuid == COMCTL_GUID_PROGRESSBAR_50 )
                xCtrlModel.reset( new ComCtlProgressBarModel( 5 ) );
            else if( *pGuid == COMCTL_GUID_PROGRESSBAR_60 )
                xCtrlModel.reset( new ComCtlProgressBarModel( 6 ) );
        }
    }

    if( xCtrlModel.get() )
    {
        // user form controls are AWT models
        xCtrlModel->setAwtModelMode();

        // check that container model matches container flag in site data
        bool bModelIsContainer = dynamic_cast< const AxContainerModelBase* >( xCtrlModel.get() ) != nullptr;
        bool bTypeMatch = bModelIsContainer == isContainer();
        OSL_ENSURE( bTypeMatch, "VbaSiteModel::createControlModel - container type does not match container flag" );
        if( !bTypeMatch )
            xCtrlModel.reset();
    }
    return xCtrlModel;
}

void VbaSiteModel::convertProperties( PropertyMap& rPropMap,
        const ControlConverter& rConv, ApiControlType eCtrlType, sal_Int32 nCtrlIndex ) const
{
    rPropMap.setProperty( PROP_Name, maName );
    rPropMap.setProperty( PROP_Tag, maTag );

    if( eCtrlType != API_CONTROL_DIALOG )
    {
        rPropMap.setProperty( PROP_HelpText, maToolTip );
        rPropMap.setProperty( PROP_EnableVisible, getFlag( mnFlags, VBA_SITE_VISIBLE ) );
        // we need to set the passed control index to make option button groups work
        if( (0 <= nCtrlIndex) && (nCtrlIndex <= SAL_MAX_INT16) )
            rPropMap.setProperty( PROP_TabIndex, static_cast< sal_Int16 >( nCtrlIndex ) );
        // progress bar and group box support TabIndex, but not Tabstop...
        if( (eCtrlType != API_CONTROL_PROGRESSBAR) && (eCtrlType != API_CONTROL_GROUPBOX) && (eCtrlType != API_CONTROL_FRAME) && (eCtrlType != API_CONTROL_PAGE) )
            rPropMap.setProperty( PROP_Tabstop, getFlag( mnFlags, VBA_SITE_TABSTOP ) );
        rConv.convertPosition( rPropMap, maPos );
    }
}

VbaFormControl::VbaFormControl()
{
}

VbaFormControl::~VbaFormControl()
{
}

void VbaFormControl::importModelOrStorage( BinaryInputStream& rInStrm, StorageBase& rStrg, const AxClassTable& rClassTable )
{
    if( mxSiteModel.get() )
    {
        if( mxSiteModel->isContainer() )
        {
            StorageRef xSubStrg = rStrg.openSubStorage( mxSiteModel->getSubStorageName(), false );
            OSL_ENSURE( xSubStrg.get(), "VbaFormControl::importModelOrStorage - cannot find storage for embedded control" );
            if( xSubStrg.get() )
                importStorage( *xSubStrg, rClassTable );
        }
        else if( !rInStrm.isEof() )
        {
            sal_Int64 nNextStrmPos = rInStrm.tell() + mxSiteModel->getStreamLength();
            importControlModel( rInStrm, rClassTable );
            rInStrm.seek( nNextStrmPos );
        }
    }
}

OUString VbaFormControl::getControlName() const
{
    return mxSiteModel.get() ? mxSiteModel->getName() : OUString();
}

void VbaFormControl::createAndConvert( sal_Int32 nCtrlIndex,
        const Reference< XNameContainer >& rxParentNC, const ControlConverter& rConv ) const
{
    if( rxParentNC.is() && mxSiteModel.get() && mxCtrlModel.get() ) try
    {
        // create the control model
        OUString aServiceName = mxCtrlModel->getServiceName();
        Reference< XMultiServiceFactory > xModelFactory( rxParentNC, UNO_QUERY_THROW );
        Reference< XControlModel > xCtrlModel( xModelFactory->createInstance( aServiceName ), UNO_QUERY_THROW );

        // convert all properties and embedded controls
        if( convertProperties( xCtrlModel, rConv, nCtrlIndex ) )
        {
            // insert into parent container
            const OUString& rCtrlName = mxSiteModel->getName();
            OSL_ENSURE( !rxParentNC->hasByName( rCtrlName ), "VbaFormControl::createAndConvert - multiple controls with equal name" );
            ContainerHelper::insertByName( rxParentNC, rCtrlName, Any( xCtrlModel ) );
        }
    }
    catch(const Exception& )
    {
    }
}

// protected ------------------------------------------------------------------

void VbaFormControl::importControlModel( BinaryInputStream& rInStrm, const AxClassTable& rClassTable )
{
    createControlModel( rClassTable );
    if( mxCtrlModel.get() )
        mxCtrlModel->importBinaryModel( rInStrm );
}

void VbaFormControl::importStorage( StorageBase& rStrg, const AxClassTable& rClassTable )
{
    createControlModel( rClassTable );
    AxContainerModelBase* pContainerModel = dynamic_cast< AxContainerModelBase* >( mxCtrlModel.get() );
    OSL_ENSURE( pContainerModel, "VbaFormControl::importStorage - missing container control model" );
    if( pContainerModel )
    {
        /*  Open the 'f' stream containing the model of this control and a list
            of site models for all child controls. */
        BinaryXInputStream aFStrm( rStrg.openInputStream( "f" ), true );
        OSL_ENSURE( !aFStrm.isEof(), "VbaFormControl::importStorage - missing 'f' stream" );

        /*  Read the properties of this container control and the class table
            (into the maClassTable vector) containing a list of GUIDs for
            exotic embedded controls. */
        if( !aFStrm.isEof() && pContainerModel->importBinaryModel( aFStrm ) && pContainerModel->importClassTable( aFStrm, maClassTable ) )
        {
            /*  Read the site models of all embedded controls (this fills the
                maControls vector). Ignore failure of importSiteModels() but
                try to import as much controls as possible. */
            importEmbeddedSiteModels( aFStrm );
            /*  Open the 'o' stream containing models of embedded simple
                controls. Stream may be empty or missing, if this control
                contains no controls or only container controls. */
            BinaryXInputStream aOStrm( rStrg.openInputStream( "o" ), true );

            /*  Iterate over all embedded controls, import model from 'o'
                stream (for embedded simple controls) or from the substorage
                (for embedded container controls). */
            maControls.forEachMem( &VbaFormControl::importModelOrStorage,
                ::std::ref( aOStrm ), ::std::ref( rStrg ), ::std::cref( maClassTable ) );

            // Special handling for multi-page which has non-standard
            // containment and additionally needs to re-order Page children
            if ( pContainerModel->getControlType() == API_CONTROL_MULTIPAGE )
            {
                AxMultiPageModel* pMultiPage = dynamic_cast< AxMultiPageModel* >( pContainerModel );
                if ( pMultiPage )
                {
                    BinaryXInputStream aXStrm( rStrg.openInputStream( "x" ), true );
                    pMultiPage->importPageAndMultiPageProperties( aXStrm, maControls.size() );
                }
                typedef std::unordered_map< sal_uInt32, std::shared_ptr< VbaFormControl > > IdToPageMap;
                IdToPageMap idToPage;
                AxArrayString sCaptions;

                for (auto const& control : maControls)
                {
                    auto& elem = control->mxCtrlModel;
                    if (!elem)
                    {
                        SAL_WARN("oox", "empty control model");
                        continue;
                    }
                    if (elem->getControlType() == API_CONTROL_PAGE)
                    {
                        VbaSiteModelRef xPageSiteRef = control->mxSiteModel;
                        if ( xPageSiteRef.get() )
                            idToPage[ xPageSiteRef->getId() ] = control;
                    }
                    else
                    {
                        AxTabStripModel* pTabStrip = static_cast<AxTabStripModel*>(elem.get());
                        sCaptions = pTabStrip->maItems;
                        pMultiPage->mnActiveTab = pTabStrip->mnListIndex;
                        pMultiPage->mnTabStyle = pTabStrip->mnTabStyle;
                    }
                }
                // apply caption/titles to pages

                maControls.clear();
                // need to sort the controls according to the order of the ids
                if ( sCaptions.size() == idToPage.size() )
                {
                    AxArrayString::iterator itCaption = sCaptions.begin();
                    for ( const auto& rCtrlId : pMultiPage->mnIDs )
                    {
                        IdToPageMap::iterator iter = idToPage.find( rCtrlId );
                        if ( iter != idToPage.end() )
                        {
                            AxPageModel* pPage = static_cast<AxPageModel*> ( iter->second->mxCtrlModel.get() );

                            pPage->importProperty( XML_Caption, *itCaption );
                            maControls.push_back( iter->second );
                        }
                        ++itCaption;
                    }
                }
            }
            /*  Reorder the controls (sorts all option buttons of an option
                group together), and move all children of all embedded frames
                (group boxes) to this control (UNO group boxes cannot contain
                other controls). */
            finalizeEmbeddedControls();
        }
    }
}

bool VbaFormControl::convertProperties( const Reference< XControlModel >& rxCtrlModel,
        const ControlConverter& rConv, sal_Int32 nCtrlIndex ) const
{
    if( rxCtrlModel.is() && mxSiteModel.get() && mxCtrlModel.get() )
    {
        const OUString& rCtrlName = mxSiteModel->getName();
        OSL_ENSURE( !rCtrlName.isEmpty(), "VbaFormControl::convertProperties - control without name" );
        if( !rCtrlName.isEmpty() )
        {
            // convert all properties
            PropertyMap aPropMap;
            mxSiteModel->convertProperties( aPropMap, rConv, mxCtrlModel->getControlType(), nCtrlIndex );
            rConv.bindToSources( rxCtrlModel, mxSiteModel->getControlSource(), mxSiteModel->getRowSource() );
            mxCtrlModel->convertProperties( aPropMap, rConv );
            mxCtrlModel->convertSize( aPropMap, rConv );
            PropertySet aPropSet( rxCtrlModel );
            aPropSet.setProperties( aPropMap );

            // create and convert all embedded controls
            if( !maControls.empty() ) try
            {
                Reference< XNameContainer > xCtrlModelNC( rxCtrlModel, UNO_QUERY_THROW );
                /*  Call conversion for all controls. Pass vector index as new
                    tab order to make option button groups work correctly. */
                maControls.forEachMemWithIndex( &VbaFormControl::createAndConvert,
                    ::std::cref( xCtrlModelNC ), ::std::cref( rConv ) );
            }
            catch(const Exception& )
            {
                OSL_FAIL( "VbaFormControl::convertProperties - cannot get control container interface" );
            }

            return true;
        }
    }
    return false;
}

// private --------------------------------------------------------------------

void VbaFormControl::createControlModel( const AxClassTable& rClassTable )
{
    // derived classes may have created their own control model
    if( !mxCtrlModel && mxSiteModel.get() )
        mxCtrlModel = mxSiteModel->createControlModel( rClassTable );
}

bool VbaFormControl::importSiteModel( BinaryInputStream& rInStrm )
{
    mxSiteModel.reset( new VbaSiteModel );
    return mxSiteModel->importBinaryModel( rInStrm );
}

void VbaFormControl::importEmbeddedSiteModels( BinaryInputStream& rInStrm )
{
    sal_uInt64 nAnchorPos = rInStrm.tell();
    sal_uInt32 nSiteCount, nSiteDataSize;
    nSiteCount = rInStrm.readuInt32();
    nSiteDataSize = rInStrm.readuInt32();
    sal_Int64 nSiteEndPos = rInStrm.tell() + nSiteDataSize;

    // skip the site info structure
    sal_uInt32 nSiteIndex = 0;
    while( !rInStrm.isEof() && (nSiteIndex < nSiteCount) )
    {
        rInStrm.skip( 1 ); // site depth
        sal_uInt8 nTypeCount = rInStrm.readuInt8(); // 'type-or-count' byte
        if( getFlag( nTypeCount, VBA_SITEINFO_COUNT ) )
        {
            /*  Count flag is set: the 'type-or-count' byte contains the number
                of controls in the lower bits, the type specifier follows in
                the next byte. The type specifier should always be 1 according
                to the specification. */
            rInStrm.skip( 1 );
            nSiteIndex += (nTypeCount & VBA_SITEINFO_MASK);
        }
        else
        {
            /*  Count flag is not set: the 'type-or-count' byte contains the
                type specifier of *one* control in the lower bits (this type
                should be 1, see above). */
            ++nSiteIndex;
        }
    }
    // align the stream to 32bit, relative to start of entire site info
    rInStrm.alignToBlock( 4, nAnchorPos );

    // import the site models for all embedded controls
    maControls.clear();
    bool bValid = !rInStrm.isEof();
    for( nSiteIndex = 0; bValid && (nSiteIndex < nSiteCount); ++nSiteIndex )
    {
        VbaFormControlRef xControl( new VbaFormControl );
        maControls.push_back( xControl );
        bValid = xControl->importSiteModel( rInStrm );
    }

    rInStrm.seek( nSiteEndPos );
}

void VbaFormControl::finalizeEmbeddedControls()
{
    /*  This function performs two tasks:

        1)  Reorder the controls appropriately (sort all option buttons of an
            option group together to make grouping work).
        2)  Move all children of all embedded frames (group boxes) to this
            control (UNO group boxes cannot contain other controls).
     */

    // first, sort all controls by original tab index
    ::std::sort( maControls.begin(), maControls.end(), &compareByTabIndex );

    /*  Collect the programmatical names of all embedded controls (needed to be
        able to set unused names to new dummy controls created below). Also
        collect the names of all children of embedded frames (group boxes).
        Luckily, names of controls must be unique in the entire form, not just
        in the current container. */
    VbaControlNamesSet aControlNames;
    VbaControlNameInserter aInserter( aControlNames );
    maControls.forEach( aInserter );
    for (auto const& control : maControls)
        if( control->mxCtrlModel.get() && (control->mxCtrlModel->getControlType() == API_CONTROL_GROUPBOX) )
            control->maControls.forEach( aInserter );

    /*  Reprocess the sorted list and collect all option button controls that
        are part of the same option group (determined by group name). All
        controls will be stored in a vector of vectors, that collects every
        option button group in one vector element, and other controls between
        these option groups (or leading or trailing controls) in other vector
        elements. If an option button group follows another group, a dummy
        separator control has to be inserted. */
    typedef RefVector< VbaFormControlVector > VbaFormControlVectorVector;
    VbaFormControlVectorVector aControlGroups;

    typedef RefMap< OUString, VbaFormControlVector > VbaFormControlVectorMap;
    VbaFormControlVectorMap aOptionGroups;

    typedef VbaFormControlVectorMap::mapped_type VbaFormControlVectorRef;
    bool bLastWasOptionButton = false;
    for (auto const& control : maControls)
    {
        const ControlModelBase* pCtrlModel = control->mxCtrlModel.get();

        if( const AxOptionButtonModel* pOptButtonModel = dynamic_cast< const AxOptionButtonModel* >( pCtrlModel ) )
        {
            // check if a new option group needs to be created
            const OUString& rGroupName = pOptButtonModel->getGroupName();
            VbaFormControlVectorRef& rxOptionGroup = aOptionGroups[ rGroupName ];
            if( !rxOptionGroup )
            {
                /*  If last control was an option button too, we have two
                    option groups following each other, so a dummy separator
                    control is needed. */
                if( bLastWasOptionButton )
                {
                    VbaFormControlVectorRef xDummyGroup( new VbaFormControlVector );
                    aControlGroups.push_back( xDummyGroup );
                    OUString aName = aControlNames.generateDummyName();
                    VbaFormControlRef xDummyControl( new VbaDummyFormControl( aName ) );
                    xDummyGroup->push_back( xDummyControl );
                }
                rxOptionGroup.reset( new VbaFormControlVector );
                aControlGroups.push_back( rxOptionGroup );
            }
            /*  Append the option button to the control group (which is now
                referred by the vector aControlGroups and by the map
                aOptionGroups). */
            rxOptionGroup->push_back(control);
            bLastWasOptionButton = true;
        }
        else
        {
            // open a new control group, if the last group is an option group
            if( bLastWasOptionButton || aControlGroups.empty() )
            {
                VbaFormControlVectorRef xControlGroup( new VbaFormControlVector );
                aControlGroups.push_back( xControlGroup );
            }
            // append the control to the last control group
            VbaFormControlVector& rLastGroup = *aControlGroups.back();
            rLastGroup.push_back(control);
            bLastWasOptionButton = false;

            // if control is a group box, move all its children to this control
            if( pCtrlModel && (pCtrlModel->getControlType() == API_CONTROL_GROUPBOX) )
            {
                /*  Move all embedded controls of the group box relative to the
                    position of the group box. */
                control->moveEmbeddedToAbsoluteParent();
                /*  Insert all children of the group box into the last control
                    group (following the group box). */
                rLastGroup.insert( rLastGroup.end(), control->maControls.begin(), control->maControls.end() );
                control->maControls.clear();
                // check if last control of the group box is an option button
                bLastWasOptionButton = dynamic_cast< const AxOptionButtonModel* >( rLastGroup.back()->mxCtrlModel.get() ) != nullptr;
            }
        }
    }

    // flatten the vector of vectors of form controls to a single vector
    maControls.clear();
    for (auto const& controlGroup : aControlGroups)
        maControls.insert( maControls.end(), controlGroup->begin(), controlGroup->end() );
}

void VbaFormControl::moveRelative( const AxPairData& rDistance )
{
    if( mxSiteModel.get() )
        mxSiteModel->moveRelative( rDistance );
}

void VbaFormControl::moveEmbeddedToAbsoluteParent()
{
    if( mxSiteModel.get() && !maControls.empty() )
    {
        // distance to move is equal to position of this control in its parent
        AxPairData aDistance = mxSiteModel->getPosition();

        /*  For group boxes: add half of the font height to Y position (VBA
            positions relative to frame border line, not to 'top' of frame). */
        const AxFontDataModel* pFontModel = dynamic_cast< const AxFontDataModel* >( mxCtrlModel.get() );
        if( pFontModel && (pFontModel->getControlType() == API_CONTROL_GROUPBOX) )
        {
            // convert points to 1/100 mm (1 pt = 1/72 inch = 2.54/72 cm = 2540/72 1/100 mm)
            sal_Int32 nFontHeight = static_cast< sal_Int32 >( pFontModel->getFontHeight() * 2540 / 72 );
            aDistance.second += nFontHeight / 2;
        }

        // move the embedded controls
        maControls.forEachMem( &VbaFormControl::moveRelative, ::std::cref( aDistance ) );
    }
}

bool VbaFormControl::compareByTabIndex( const VbaFormControlRef& rxLeft, const VbaFormControlRef& rxRight )
{
    // sort controls without model to the end
    sal_Int32 nLeftTabIndex = rxLeft->mxSiteModel.get() ? rxLeft->mxSiteModel->getTabIndex() : SAL_MAX_INT32;
    sal_Int32 nRightTabIndex = rxRight->mxSiteModel.get() ? rxRight->mxSiteModel->getTabIndex() : SAL_MAX_INT32;
    return nLeftTabIndex < nRightTabIndex;
}

namespace {

OUString lclGetQuotedString( const OUString& rCodeLine )
{
    OUStringBuffer aBuffer;
    sal_Int32 nLen = rCodeLine.getLength();
    if( (nLen > 0) && (rCodeLine[ 0 ] == '"') )
    {
        bool bExitLoop = false;
        for( sal_Int32 nIndex = 1; !bExitLoop && (nIndex < nLen); ++nIndex )
        {
            sal_Unicode cChar = rCodeLine[ nIndex ];
            // exit on closing quote char (but check on double quote chars)
            bExitLoop = (cChar == '"') && ((nIndex + 1 == nLen) || (rCodeLine[ nIndex + 1 ] != '"'));
            if( !bExitLoop )
            {
                aBuffer.append( cChar );
                // skip second quote char
                if( cChar == '"' )
                    ++nIndex;
            }
        }
    }
    return aBuffer.makeStringAndClear();
}

bool lclEatWhitespace( OUString& rCodeLine )
{
    sal_Int32 nIndex = 0;
    while( (nIndex < rCodeLine.getLength()) && ((rCodeLine[ nIndex ] == ' ') || (rCodeLine[ nIndex ] == '\t')) )
        ++nIndex;
    if( nIndex > 0 )
    {
        rCodeLine = rCodeLine.copy( nIndex );
        return true;
    }
    return false;
}

bool lclEatKeyword( OUString& rCodeLine, const OUString& rKeyword )
{
    if( rCodeLine.matchIgnoreAsciiCase( rKeyword ) )
    {
        rCodeLine = rCodeLine.copy( rKeyword.getLength() );
        // success, if code line ends after keyword, or if whitespace follows
        return rCodeLine.isEmpty() || lclEatWhitespace( rCodeLine );
    }
    return false;
}

} // namespace

VbaUserForm::VbaUserForm( const Reference< XComponentContext >& rxContext,
        const Reference< XModel >& rxDocModel, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    mxContext( rxContext ),
    mxDocModel( rxDocModel ),
    maConverter( rxDocModel, rGraphicHelper, bDefaultColorBgr )
{
    OSL_ENSURE( mxContext.is(), "VbaUserForm::VbaUserForm - missing component context" );
    OSL_ENSURE( mxDocModel.is(), "VbaUserForm::VbaUserForm - missing document model" );
}

void VbaUserForm::importForm( const Reference< XNameContainer >& rxDialogLib,
                           StorageBase& rVbaFormStrg, const OUString& rModuleName, rtl_TextEncoding eTextEnc )
{
    OSL_ENSURE( rxDialogLib.is(), "VbaUserForm::importForm - missing dialog library" );
    if( !mxContext.is() || !mxDocModel.is() || !rxDialogLib.is() )
        return;

    // check that the '03VBFrame' stream exists, this is required for forms
    BinaryXInputStream aInStrm( rVbaFormStrg.openInputStream( "\003VBFrame" ), true );
    OSL_ENSURE( !aInStrm.isEof(), "VbaUserForm::importForm - missing \\003VBFrame stream" );
    if( aInStrm.isEof() )
        return;

    // scan for the line 'Begin {GUID} <FormName>'
    TextInputStream aFrameTextStrm( mxContext, aInStrm, eTextEnc );
    const OUString aBegin = "Begin";
    OUString aLine;
    bool bBeginFound = false;
    while( !bBeginFound && !aFrameTextStrm.isEof() )
    {
        aLine = aFrameTextStrm.readLine().trim();
        bBeginFound = lclEatKeyword( aLine, aBegin );
    }
    // check for the specific GUID that represents VBA forms
    if( !bBeginFound || !lclEatKeyword( aLine, "{C62A69F0-16DC-11CE-9E98-00AA00574A4F}" ) )
        return;

    // remaining line is the form name
    OUString aFormName = aLine.trim();
    OSL_ENSURE( !aFormName.isEmpty(), "VbaUserForm::importForm - missing form name" );
    OSL_ENSURE( rModuleName.equalsIgnoreAsciiCase( aFormName ), "VbaUserForm::importFrameStream - form and module name mismatch" );
    if( aFormName.isEmpty() )
        aFormName = rModuleName;
    if( aFormName.isEmpty() )
        return;
    mxSiteModel.reset( new VbaSiteModel );
    mxSiteModel->importProperty( XML_Name, aFormName );

    // read the form properties (caption is contained in this '03VBFrame' stream, not in the 'f' stream)
    mxCtrlModel.reset( new AxUserFormModel );
    OUString aKey, aValue;
    bool bExitLoop = false;
    while( !bExitLoop && !aFrameTextStrm.isEof() )
    {
        aLine = aFrameTextStrm.readLine().trim();
        bExitLoop = aLine.equalsIgnoreAsciiCase( "End" );
        if( !bExitLoop && VbaHelper::extractKeyValue( aKey, aValue, aLine ) )
        {
            if( aKey.equalsIgnoreAsciiCase( "Caption" ) )
                mxCtrlModel->importProperty( XML_Caption, lclGetQuotedString( aValue ) );
            else if( aKey.equalsIgnoreAsciiCase( "Tag" ) )
                mxSiteModel->importProperty( XML_Tag, lclGetQuotedString( aValue ) );
        }
    }

    // use generic container control functionality to import the embedded controls
    importStorage( rVbaFormStrg, AxClassTable() );

    try
    {
        // create the dialog model
        OUString aServiceName = mxCtrlModel->getServiceName();
        Reference< XMultiServiceFactory > xFactory( mxContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< XControlModel > xDialogModel( xFactory->createInstance( aServiceName ), UNO_QUERY_THROW );
        Reference< XNameContainer > xDialogNC( xDialogModel, UNO_QUERY_THROW );

        // convert properties and embedded controls
        if( convertProperties( xDialogModel, maConverter, 0 ) )
        {
            // export the dialog to XML and insert it into the dialog library
            Reference< XInputStreamProvider > xDialogSource( ::xmlscript::exportDialogModel( xDialogNC, mxContext, mxDocModel ), UNO_SET_THROW );
            OSL_ENSURE( !rxDialogLib->hasByName( aFormName ), "VbaUserForm::importForm - multiple dialogs with equal name" );
            ContainerHelper::insertByName( rxDialogLib, aFormName, Any( xDialogSource ) );
        }
    }
    catch(const Exception& )
    {
    }
}

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
