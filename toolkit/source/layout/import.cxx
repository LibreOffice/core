#include "import.hxx"

#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <tools/debug.hxx>

#include "root.hxx"
#include "helper.hxx"
#include "dialogbuttonhbox.hxx"


#define XMLNS_LAYOUT_URI    "http://openoffice.org/2007/layout"
#define XMLNS_CONTAINER_URI "http://openoffice.org/2007/layout/container"

namespace layoutimpl
{
using namespace css;

using ::rtl::OUString;

ElementBase::~ElementBase()
SAL_THROW( () )
{
    //delete mpImport;
    //mpImport = 0;
}

//** parser
WidgetElement::WidgetElement ( sal_Int32 nUid, const OUString &rName,
                               uno::Reference <xml::input::XAttributes> const &attributes,
                               ElementBase *pParent,
                               ImportContext *pImport)
SAL_THROW (())
: ElementBase( nUid, rName, attributes, pParent, pImport )
{
    OUString name = rName.toAsciiLowerCase();

    PropList aProps;
    propsFromAttributes( attributes, aProps, pImport->XMLNS_LAYOUT_UID );

    OUString aId;
    findAndRemove( "id", aProps, aId );
    OUString aLang;
    findAndRemove( "xml-lang", aProps, aLang );

    {
//DEBUG
        uno::Reference< awt::XLayoutConstrains > xParent;
        if ( pParent )
            xParent = ((WidgetElement *) pParent)->mpWidget->getPeer();


        mpWidget = pImport->mrRoot.create( aId, name,
                                           getAttributeProps( aProps ), uno::Reference< awt::XLayoutContainer >( xParent, uno::UNO_QUERY ) );

    }

    // TODO: handle with non-existing widgets

    mpWidget->setProperties( aProps );

    uno::Reference< awt::XDialog2 > xDialog( mpWidget->getPeer(), uno::UNO_QUERY );
    if ( xDialog.is() )
    {
        OUString aTitle;
        if ( findAndRemove( "title", aProps, aTitle ) )
            xDialog->setTitle( aTitle );
    } // DEBUG:
    else if ( pParent == NULL )
    {
        DBG_ERROR( "Fatal error: top node isn't a dialog" );
    }

    bool bOrdering;
    OUString aOrdering;
    bOrdering = findAndRemove( "ordering", aProps, aOrdering );
    if ( DialogButtonHBox *b = dynamic_cast<DialogButtonHBox *> ( mpWidget->getPeer().get() ) )
    {
        if ( bOrdering )
            b->setOrdering ( aOrdering );
    }

#ifdef IMPORT_RADIOGROUP
    bool bSetRadioGroup, bSetTitle;
    OUString aRadioGroup, aTitle;
    bSetRadioGroup = findAndRemove( "radiogroup", aProps, aRadioGroup );
    bSetTitle = findAndRemove( "title", aProps, aTitle );

    setProperties( mxPeer, aProps );

    // we need to add radio buttons to the group after their properties are
    // set, so we can check if they should be the one selected by default or not.
    // And the state changed event isn't fired when changing properties.
    if ( bSetRadioGroup )
    {
        static int i = 0;
        i++;
        uno::Reference< awt::XRadioButton > xRadio( mxPeer, uno::UNO_QUERY );
        if ( xRadio.is() )
            pImport->mxRadioGroups.addItem( aRadioGroup, xRadio );
    }
    if ( bSetTitle )
    {
        uno::Reference< awt::XDialog2 > xDialog( mxPeer, uno::UNO_QUERY );
        if ( xDialog.is() )
            xDialog->setTitle( aTitle );
    }
#endif
}

WidgetElement::~WidgetElement()
{
    //delete mpWidget;
    //mpWidget = 0;
}

uno::Reference <xml::input::XElement>
WidgetElement::startChildElement ( sal_Int32 nUid, OUString const &name,
                                   uno::Reference <xml::input::XAttributes> const &attributes )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    // Adding a child to the widget
    WidgetElement *pChild = new WidgetElement ( nUid, name, attributes, this, mpImport );

    if ( !mpWidget->addChild( pChild->mpWidget ) )
    {
        DBG_ERROR2( "ERROR: cannot add %s to container %s, container full", OUSTRING_CSTR( name ), OUSTRING_CSTR( getLocalName() ) );
        throw xml::sax::SAXException();
    }

    PropList aProps;
    propsFromAttributes( attributes, aProps, mpImport->XMLNS_CONTAINER_UID );
    mpWidget->setChildProperties( pChild->mpWidget, aProps );

    return pChild;
}

// Support Ivo Hinkelmann's move label/text/title attribute to CONTENT
// transex3 hack.
void SAL_CALL
WidgetElement::characters( OUString const& rChars )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException)
{
    if ( mpWidget && rChars.trim().getLength() )
    {
        uno::Reference< awt::XDialog2 > xDialog( mpWidget->getPeer(), uno::UNO_QUERY );
        uno::Reference< awt::XButton > xButton( mpWidget->getPeer(), uno::UNO_QUERY );
        if ( xDialog.is() )
            xDialog->setTitle( rChars );
        else if ( xButton.is() )
            mpWidget->setProperty( OUString::createFromAscii( "label" ), rChars );
        else
            mpWidget->setProperty( OUString::createFromAscii( "text" ), rChars );
    }
}
// ---- ElementBase ----

ElementBase::ElementBase( sal_Int32 nUid, OUString const & rLocalName,
                          uno::Reference< xml::input::XAttributes > const & xAttributes,
                          ElementBase* pParent,
                          ImportContext* pImport )
SAL_THROW(())
: mpImport( pImport )
    , mpParent( pParent )
    , mnUid( nUid )
    , maLocalName( rLocalName )
    , mxAttributes( xAttributes )
{
}

// ---- ImportContext ----

void ImportContext::startDocument(
    uno::Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    XMLNS_LAYOUT_UID = xNamespaceMapping->getUidByUri(
        OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_LAYOUT_URI ) ) );
    XMLNS_CONTAINER_UID = xNamespaceMapping->getUidByUri(
        OUString( RTL_CONSTASCII_USTRINGPARAM( XMLNS_CONTAINER_URI ) ) );
}

ToplevelElement::ToplevelElement (OUString const &rName,
                                  uno::Reference <xml::input::XAttributes> const &xAttributes,
                                  ImportContext *pImport)
SAL_THROW(())
: WidgetElement( 0, rName, xAttributes, NULL, pImport )
{
}

ToplevelElement::~ToplevelElement()
{
}

uno::Reference< xml::input::XElement > ImportContext::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    uno::Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, uno::RuntimeException)
{
    if (XMLNS_LAYOUT_UID != nUid)
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "invalid namespace!" ) ),
            uno::Reference< uno::XInterface >(), uno::Any() );
    else
        return new ToplevelElement( rLocalName, xAttributes, this );
}

} // namespace layoutimpl
