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

#include "oox/ole/vbacontrol.hxx"

#include <algorithm>
#include <set>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustrbuf.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/helper/textinputstream.hxx"
#include "oox/ole/vbahelper.hxx"

namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

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
const sal_uInt32 VBA_SITE_DEFAULTBUTTON         = 0x00000004;
const sal_uInt32 VBA_SITE_CANCELBUTTON          = 0x00000008;
const sal_uInt32 VBA_SITE_OSTREAM               = 0x00000010;
const sal_uInt32 VBA_SITE_DEFFLAGS              = 0x00000033;

const sal_uInt8 VBA_SITEINFO_COUNT              = 0x80;
const sal_uInt8 VBA_SITEINFO_MASK               = 0x7F;

// ----------------------------------------------------------------------------

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
    typedef ::std::set< OUString > OUStringSet;
    OUStringSet         maCtrlNames;
    const OUString      maDummyBaseName;
    sal_Int32           mnIndex;
};

VbaControlNamesSet::VbaControlNamesSet() :
    maDummyBaseName( CREATE_OUSTRING( "DummyGroupSep" ) ),
    mnIndex( 0 )
{
}

void VbaControlNamesSet::insertName( const VbaFormControl& rControl )
{
    OUString aName = rControl.getControlName();
    if( aName.getLength() > 0 )
        maCtrlNames.insert( aName );
}

OUString VbaControlNamesSet::generateDummyName()
{
    OUString aCtrlName;
    do
    {
        aCtrlName = OUStringBuffer( maDummyBaseName ).append( ++mnIndex ).makeStringAndClear();
    }
    while( maCtrlNames.count( aCtrlName ) > 0 );
    maCtrlNames.insert( aCtrlName );
    return aCtrlName;
}

// ----------------------------------------------------------------------------

/** Functor that inserts the name of a control into a VbaControlNamesSet. */
struct VbaControlNameInserter
{
public:
    VbaControlNamesSet& mrCtrlNames;
    inline explicit     VbaControlNameInserter( VbaControlNamesSet& rCtrlNames ) : mrCtrlNames( rCtrlNames ) {}
    inline void         operator()( const VbaFormControl& rControl ) { mrCtrlNames.insertName( rControl ); }
};

// ----------------------------------------------------------------------------

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
    mxSiteModel->importProperty( XML_VariousPropertyBits, OUString( sal_Unicode( '0' ) ) );

    mxCtrlModel.reset( new AxLabelModel );
    mxCtrlModel->setAwtModelMode();
    mxCtrlModel->importProperty( XML_Size, CREATE_OUSTRING( "10;10" ) );
}

} // namespace

// ============================================================================

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

bool VbaSiteModel::isVisible() const
{
    return getFlag( mnFlags, VBA_SITE_VISIBLE );
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
        aBuffer.append( sal_Unicode( 'i' ) );
        if( mnId < 10 )
            aBuffer.append( sal_Unicode( '0' ) );
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
            case VBA_SITE_TABSTRIP:         xCtrlModel.reset( new AxTabStripModel );        break;
            case VBA_SITE_FRAME:            xCtrlModel.reset( new AxFrameModel );           break;
            case VBA_SITE_MULTIPAGE:        xCtrlModel.reset( new AxMultiPageModel );       break;
            case VBA_SITE_FORM:             xCtrlModel.reset( new AxFormPageModel );        break;
            default:    OSL_FAIL( "VbaSiteModel::createControlModel - unknown type index" );
        }
    }
    else
    {
        const OUString* pGuid = ContainerHelper::getVectorElement( rClassTable, nTypeIndex );
        OSL_ENSURE( pGuid, "VbaSiteModel::createControlModel - invalid class table index" );
        if( pGuid )
        {
            if( pGuid->equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(COMCTL_GUID_SCROLLBAR_60)) )
                xCtrlModel.reset( new ComCtlScrollBarModel( 6 ) );
            else if( pGuid->equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(COMCTL_GUID_PROGRESSBAR_50)) )
                xCtrlModel.reset( new ComCtlProgressBarModel( 5 ) );
            else if( pGuid->equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(COMCTL_GUID_PROGRESSBAR_60)) )
                xCtrlModel.reset( new ComCtlProgressBarModel( 6 ) );
        }
    }

    if( xCtrlModel.get() )
    {
        // user form controls are AWT models
        xCtrlModel->setAwtModelMode();

        // check that container model matches container flag in site data
        bool bModelIsContainer = dynamic_cast< const AxContainerModelBase* >( xCtrlModel.get() ) != 0;
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

void VbaSiteModel::bindToSources( const Reference< XControlModel >& rxCtrlModel, const ControlConverter& rConv ) const
{
    rConv.bindToSources( rxCtrlModel, maControlSource, maRowSource );
}

// ============================================================================

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

sal_Int32 VbaFormControl::getControlId() const
{
    return mxSiteModel.get() ? mxSiteModel->getId() : -1;
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
    catch( Exception& )
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
        BinaryXInputStream aFStrm( rStrg.openInputStream( CREATE_OUSTRING( "f" ) ), true );
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
            BinaryXInputStream aOStrm( rStrg.openInputStream( CREATE_OUSTRING( "o" ) ), true );

            /*  Iterate over all embedded controls, import model from 'o'
                stream (for embedded simple controls) or from the substorage
                (for embedded container controls). */
            maControls.forEachMem( &VbaFormControl::importModelOrStorage,
                ::boost::ref( aOStrm ), ::boost::ref( rStrg ), ::boost::cref( maClassTable ) );

            /** Final processing on the control and all embedded controls,
                depending on the type of this control. */
            finalizeEmbeddedControls( rStrg );
        }
    }
}

bool VbaFormControl::convertProperties( const Reference< XControlModel >& rxCtrlModel,
        const ControlConverter& rConv, sal_Int32 nCtrlIndex ) const
{
    if( rxCtrlModel.is() && mxSiteModel.get() && mxCtrlModel.get() )
    {
        const OUString& rCtrlName = mxSiteModel->getName();
        OSL_ENSURE( rCtrlName.getLength() > 0, "VbaFormControl::convertProperties - control without name" );
        if( rCtrlName.getLength() > 0 )
        {
            // convert all properties
            PropertyMap aPropMap;
            mxSiteModel->convertProperties( aPropMap, rConv, mxCtrlModel->getControlType(), nCtrlIndex );
            mxCtrlModel->convertProperties( aPropMap, rConv );
            mxCtrlModel->convertSize( aPropMap, rConv );
            PropertySet aPropSet( rxCtrlModel );
            aPropSet.setProperties( aPropMap );

            // bind to control source and row source range
            mxSiteModel->bindToSources( rxCtrlModel, rConv );

            // create and convert all embedded controls
            if( !maControls.empty() ) try
            {
                Reference< XNameContainer > xCtrlModelNC( rxCtrlModel, UNO_QUERY_THROW );
                /*  Call conversion for all controls. Pass vector index as new
                    tab order to make option button groups work correctly. */
                maControls.forEachMemWithIndex( &VbaFormControl::createAndConvert,
                    ::boost::cref( xCtrlModelNC ), ::boost::cref( rConv ) );
            }
            catch( Exception& )
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

bool VbaFormControl::importEmbeddedSiteModels( BinaryInputStream& rInStrm )
{
    sal_uInt64 nAnchorPos = rInStrm.tell();
    sal_uInt32 nSiteCount, nSiteDataSize;
    rInStrm >> nSiteCount >> nSiteDataSize;
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
    return bValid;
}

void VbaFormControl::finalizeEmbeddedControls( StorageBase& rStrg )
{
    /*  Store all embedded controls in a temporary vector, so "exit on error"
        will leave this control empty. */
    VbaFormControlVector aControls;
    aControls.swap( maControls );

    /*  If this is a multipage control, it stores additional data in the 'x'
        stream of its storage. It contains the control identifiers of the form
        page controls that contain the embedded controls of each page.
        Additionally, the order of these pages is stored there (they are not
        nessecarily in the order they are persisted in). */
    if( AxMultiPageModel* pMultiPageModel = dynamic_cast< AxMultiPageModel* >( mxCtrlModel.get() ) )
    {
        // read additional attributes from the 'x' stream
        BinaryXInputStream aXStrm( rStrg.openInputStream( CREATE_OUSTRING( "x" ) ), true );
        OSL_ENSURE( !aXStrm.isEof(), "VbaFormControl::finalizeEmbeddedControls - missing 'x' stream" );
        if( aXStrm.isEof() ) return;

        // skip the page property structures related to all controls
        for( size_t nSiteIdx = 0, nSiteCount = aControls.size(); nSiteIdx < nSiteCount; ++nSiteIdx )
        {
            AxBinaryPropertyReader aReader( aXStrm );
            aReader.skipUndefinedProperty();
            aReader.skipIntProperty< sal_uInt32 >(); // transition effect
            aReader.skipIntProperty< sal_uInt32 >(); // transition period
            if( !aReader.finalizeImport() ) return;
        }

        // read the multipage property structure containing a list of page IDs
        sal_Int32 nPageCount = 0;
        sal_Int32 nTabStripId = 0;
        AxBinaryPropertyReader aReader( aXStrm );
        aReader.skipUndefinedProperty();
        aReader.readIntProperty< sal_Int32 >( nPageCount );
        aReader.readIntProperty< sal_Int32 >( nTabStripId );
        if( !aReader.finalizeImport() ) return;
        // read the array containing all page identifiers in current order
        typedef ::std::vector< sal_Int32 > AxPageIdVector;
        AxPageIdVector aPageIds;
        for( sal_Int32 nPage = 0; !aXStrm.isEof() && (nPage < nPageCount); ++nPage )
            aPageIds.push_back( aXStrm.readInt32() );
        if( aXStrm.isEof() ) return;

        // check the page count value
        bool bValidPageCount = (0 < nPageCount) && (static_cast< size_t >( nPageCount + 1 ) == aControls.size());
        OSL_ENSURE( bValidPageCount, "VbaFormControl::finalizeEmbeddedControls - invalid number of pages" );
        if( !bValidPageCount ) return;

        /*  Check that this multipage contains the expected controls:
            - a tabstrip control, specified by nTabStripId,
            - form page controls (containing the embedded controls of each page). */

        // the controls may be in arbitrary order, first map them by ID
        RefMap< sal_Int32, VbaFormControl > aControlsById;
        for( VbaFormControlVector::iterator aIt = aControls.begin(), aEnd = aControls.end(); aIt != aEnd; ++aIt )
        {
            VbaFormControlRef xControl = *aIt;
            sal_Int32 nId = xControl->getControlId();
            OSL_ENSURE( (nId > 0) && !aControlsById.has( nId ), "VbaFormControl::finalizeEmbeddedControls - invalid control ID" );
            aControlsById[ nId ] = xControl;
        }
        // store tabstrip in the multipage, it will care about property conversion
        AxTabStripModelRef xTabStripModel;
        VbaFormControlRef xControl = aControlsById.get( nTabStripId );
        if( xControl.get() )
            xTabStripModel = ::boost::dynamic_pointer_cast< AxTabStripModel >( xControl->mxCtrlModel );
        OSL_ENSURE( xTabStripModel.get(), "VbaFormControl::finalizeEmbeddedControls - missing tabstrip control" );
        if( !xTabStripModel ) return;
        pMultiPageModel->setTabStripModel( xTabStripModel );
        aControlsById.erase( nTabStripId );
        // store all pages in maControls in the correct order specified by aPageIds
        sal_Int32 nTabIndex = 0;
        for( AxPageIdVector::iterator aIt = aPageIds.begin(), aEnd = aPageIds.end(); aIt != aEnd; ++aIt, ++nTabIndex )
        {
            VbaFormControlRef rControl = aControlsById.get( *aIt );
            AxFormPageModel* pFormPageModel = rControl.get() ? dynamic_cast< AxFormPageModel* >( rControl->mxCtrlModel.get() ) : 0;
            OSL_ENSURE( pFormPageModel, "VbaFormControl::finalizeEmbeddedControls - missing formpage control" );
            // do not exit on error but try to collect as much pages as possible
            if( pFormPageModel )
            {
                // get the tab caption from tabstrip control and set it at the formpage
                OUString aCaption = xTabStripModel->getCaption( nTabIndex );
                pFormPageModel->importProperty( XML_Caption, aCaption );
                // store the control in maControls
                maControls.push_back( rControl );
                aControlsById.erase( *aIt );
            }
        }
    }
    else
    {
        /*  Reorder the controls appropriately (sort all option buttons of an
            option group together to make grouping work), and erase all plain
            tabstrip controls (currently not supported in UNO dialogs). */

        // first, sort all controls by original tab index
        ::std::sort( aControls.begin(), aControls.end(), &compareByTabIndex );

        /*  Collect the programmatical names of all embedded controls (needed to be
            able to set unused names to new dummy controls created below). */
        VbaControlNamesSet aControlNames;
        VbaControlNameInserter aInserter( aControlNames );
        aControls.forEach( aInserter );

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
        for( VbaFormControlVector::iterator aIt = aControls.begin(), aEnd = aControls.end(); aIt != aEnd; ++aIt )
        {
            VbaFormControlRef xControl = *aIt;
            const ControlModelBase* pCtrlModel = xControl->mxCtrlModel.get();
            if ( !pCtrlModel ) // skip unsupported controls
                continue;
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
                rxOptionGroup->push_back( xControl );
                bLastWasOptionButton = true;
            }
            else
            {
                // skip unsupported controls (tabstrips and page controls)
                ApiControlType eCtrlType = pCtrlModel->getControlType();
                if( (eCtrlType != API_CONTROL_TABSTRIP) && (eCtrlType != API_CONTROL_PAGE) )
                {
                    // open a new control group, if the last group is an option group
                    if( bLastWasOptionButton || aControlGroups.empty() )
                    {
                        VbaFormControlVectorRef xControlGroup( new VbaFormControlVector );
                        aControlGroups.push_back( xControlGroup );
                    }
                    // append the control to the last control group
                    VbaFormControlVector& rLastGroup = *aControlGroups.back();
                    rLastGroup.push_back( xControl );
                    bLastWasOptionButton = false;
                }
            }
        }

        // flatten the vector of vectors of form controls to a single vector
        for( VbaFormControlVectorVector::iterator aIt = aControlGroups.begin(), aEnd = aControlGroups.end(); aIt != aEnd; ++aIt )
            maControls.insert( maControls.end(), (*aIt)->begin(), (*aIt)->end() );
    }
}

/*static*/ bool VbaFormControl::compareByTabIndex( const VbaFormControlRef& rxLeft, const VbaFormControlRef& rxRight )
{
    // sort controls without model to the end
    sal_Int32 nLeftTabIndex = rxLeft->mxSiteModel.get() ? rxLeft->mxSiteModel->getTabIndex() : SAL_MAX_INT32;
    sal_Int32 nRightTabIndex = rxRight->mxSiteModel.get() ? rxRight->mxSiteModel->getTabIndex() : SAL_MAX_INT32;
    return nLeftTabIndex < nRightTabIndex;
}

// ============================================================================

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
        return (rCodeLine.getLength() == 0) || lclEatWhitespace( rCodeLine );
    }
    return false;
}

} // namespace

// ----------------------------------------------------------------------------

VbaUserForm::VbaUserForm( const Reference< XComponentContext >& rxContext,
        const Reference< XModel >& rxDocModel, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr ) :
    mxCompContext( rxContext ),
    mxDocModel( rxDocModel ),
    maConverter( rxDocModel, rGraphicHelper, bDefaultColorBgr )
{
    OSL_ENSURE( mxCompContext.is(), "VbaUserForm::VbaUserForm - missing component context" );
    OSL_ENSURE( mxDocModel.is(), "VbaUserForm::VbaUserForm - missing document model" );
}

void VbaUserForm::importForm(
                           const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxDocModel,
                           const Reference< XNameContainer >& rxDialogLib,
                           StorageBase& rVbaFormStrg, const OUString& rModuleName, rtl_TextEncoding eTextEnc )
{
    OSL_ENSURE( rxDialogLib.is(), "VbaUserForm::importForm - missing dialog library" );
    if( !mxCompContext.is() || !mxDocModel.is() || !rxDialogLib.is() )
        return;

    // check that the '03VBFrame' stream exists, this is required for forms
    BinaryXInputStream aInStrm( rVbaFormStrg.openInputStream( CREATE_OUSTRING( "\003VBFrame" ) ), true );
    OSL_ENSURE( !aInStrm.isEof(), "VbaUserForm::importForm - missing \\003VBFrame stream" );
    if( aInStrm.isEof() )
        return;

    // scan for the line 'Begin {GUID} <FormName>'
    TextInputStream aFrameTextStrm( aInStrm, eTextEnc );
    const OUString aBegin = CREATE_OUSTRING( "Begin" );
    OUString aLine;
    bool bBeginFound = false;
    while( !bBeginFound && !aFrameTextStrm.isEof() )
    {
        aLine = aFrameTextStrm.readLine().trim();
        bBeginFound = lclEatKeyword( aLine, aBegin );
    }
    // check for the specific GUID that represents VBA forms
    if( !bBeginFound || !lclEatKeyword( aLine, CREATE_OUSTRING( "{C62A69F0-16DC-11CE-9E98-00AA00574A4F}" ) ) )
        return;

    // remaining line is the form name
    OUString aFormName = aLine.trim();
    OSL_ENSURE( aFormName.getLength() > 0, "VbaUserForm::importForm - missing form name" );
    OSL_ENSURE( rModuleName.equalsIgnoreAsciiCase( aFormName ), "VbaUserForm::importFrameStream - form and module name mismatch" );
    if( aFormName.getLength() == 0 )
        aFormName = rModuleName;
    if( aFormName.getLength() == 0 )
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
        bExitLoop = aLine.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "End" ) );
        if( !bExitLoop && VbaHelper::extractKeyValue( aKey, aValue, aLine ) )
        {
            if( aKey.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Caption" ) ) )
                mxCtrlModel->importProperty( XML_Caption, lclGetQuotedString( aValue ) );
            else if( aKey.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "Tag" ) ) )
                mxSiteModel->importProperty( XML_Tag, lclGetQuotedString( aValue ) );
        }
    }

    // use generic container control functionality to import the embedded controls
    importStorage( rVbaFormStrg, AxClassTable() );

    try
    {
        // create the dialog model
        OUString aServiceName = mxCtrlModel->getServiceName();
        Reference< XMultiServiceFactory > xFactory( mxCompContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< XControlModel > xDialogModel( xFactory->createInstance( aServiceName ), UNO_QUERY_THROW );
        Reference< XNameContainer > xDialogNC( xDialogModel, UNO_QUERY_THROW );

        // convert properties and embedded controls
        if( convertProperties( xDialogModel, maConverter, 0 ) )
        {
            // export the dialog to XML and insert it into the dialog library
            Reference< XInputStreamProvider > xDialogSource( ::xmlscript::exportDialogModel( xDialogNC, mxCompContext, rxDocModel ), UNO_SET_THROW );
            OSL_ENSURE( !rxDialogLib->hasByName( aFormName ), "VbaUserForm::importForm - multiple dialogs with equal name" );
            ContainerHelper::insertByName( rxDialogLib, aFormName, Any( xDialogSource ) );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
