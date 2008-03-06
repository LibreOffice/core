#ifndef IMPORT_HXX
#define IMPORT_HXX

#include <map>
#include <list>
#include <hash_map>

#include <com/sun/star/xml/input/XRoot.hpp>
#include <cppuhelper/implbase1.hxx>

namespace layoutimpl
{
class LayoutRoot;
class LayoutWidget;
namespace css = ::com::sun::star;

/* blocks under IMPORT_RADIOGROUP are marked for deletion.
   The use of it is to synchronize radio buttons into groups.
   But toolkit doesn't fire toggle events when toggled from the code.
   Another approach is to implement our own XRadioButton from our
   internal toolkit. We could have some singleton where they would
   register... We would need to add another attribute...
*/

#ifdef IMPORT_RADIOGROUP
#include <com/sun/star/awt/XRadioButton.hpp>
class RadioGroups
{
public:
    RadioGroups()
    {
    }

    void addItem( rtl::OUString id, css::uno::Reference< css::awt::XRadioButton > xRadio )
        throw (css::uno::RuntimeException)
    {
        if ( ! xRadio.is() )
            throw css::uno::RuntimeException();

        css::uno::Reference< RadioGroup > group;
        RadioGroupsMap::iterator it = mxRadioGroups.find( id );
        if ( it == mxRadioGroups.end() )
        {
            group = css::uno::Reference< RadioGroup > ( new RadioGroup() );
            mxRadioGroups [id] = group;
        }
        else
            group = it->second;
        group->addItem( xRadio );
    }

private:
    class RadioGroup : public ::cppu::WeakImplHelper1< css::awt::XItemListener >
    {
    public:
        RadioGroup()
        {
        }

        void addItem( css::uno::Reference< css::awt::XRadioButton > xRadio )
        {
            if ( ! mxSelectedRadio.is() )
            {
                xRadio->setState( true );
                mxSelectedRadio = xRadio;
            }
            else if ( xRadio->getState() )
            {
                mxSelectedRadio->setState( false );
                mxSelectedRadio = xRadio;
            }

            xRadio->addItemListener( this );
            mxRadios.push_back (xRadio);
        }

    private:
        typedef std::list< css::uno::Reference< css::awt::XRadioButton > > RadioButtonsList;
        RadioButtonsList mxRadios;
        css::uno::Reference< css::awt::XRadioButton > mxSelectedRadio;

        // awt::XItemListener
        void itemStateChanged( const com::sun::star::awt::ItemEvent& e)
            throw (css::uno::RuntimeException)
        {
            if ( e.Selected )
            {
                mxSelectedRadio->setState( false );
                // the only radio button selected would be the one that fired the event
                for( RadioButtonsList::iterator it = mxRadios.begin(); it != mxRadios.end(); it++ )
                    if ( (*it)->getState() )
                    {
                        mxSelectedRadio = *it;
                        break;
                    }
            }
        }

        // lang::XEventListener
        void SAL_CALL disposing( const css::lang::EventObject& )
            throw (css::uno::RuntimeException)
        {
        }
    };

    // each RadioGroup will stay alive after RadioGroups die with the ImportContext
    // because they are referenced by every XRadioButton through the listener
    typedef std::map< rtl::OUString, css::uno::Reference< RadioGroup > > RadioGroupsMap;
    RadioGroupsMap mxRadioGroups;
};
#endif

#if 0
// generator
class Widget
{
public:
    Widget( css::uno::Reference< css::awt::XToolkit > xToolkit,
            css::uno::Reference< css::awt::XWindow > xToplevel,
            rtl::OUString unoName, long attrbs );
    virtual ~Widget();

    virtual void setProperties( const PropList &rProps );

    virtual bool addChild( Widget *pChild );
    virtual void setChildProperties( Widget *pChild, const PropList &rProps );

    inline css::uno::Reference< css::awt::XLayoutConstrains > getPeer()
    { return mxWidget; }

    inline css::uno::Reference< css::awt::XLayoutConstrains > getContainer()
    { return mxContainer; }

protected:
    css::uno::Reference< css::awt::XLayoutConstrains > mxWidget;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
};

class Root
{
public:
    Root( css::uno::Reference< css::awt::XToolkit > xToolkit )
        : mxToolkit( xToolkit ) {}
    ~Root();

    virtual Widget *create( rtl::OUString id, const rtl::OUString unoName, long attrbs );

    css::uno::Reference< css::awt::XLayoutConstrains > getById( rtl::OUString id );
    inline css::uno::Reference< css::awt::XLayoutConstrains > getToplevel();

protected:
    css::uno::Reference< css::awt::XToolkit > mxToolkit;
    Widget *mpToplevel;

    typedef std::hash_map< rtl::OUString, css::uno::Reference< css::awt::XLayoutConstrains >,
                           rtl::OUStringHash > ItemHash;
    ItemHash maItems;
};
#endif

// parser
class ImportContext : public ::cppu::WeakImplHelper1< css::xml::input::XRoot >
{
public:
    sal_Int32 XMLNS_LAYOUT_UID, XMLNS_CONTAINER_UID;
    LayoutRoot &mrRoot; // switch to XNameContainer ref ?
#ifdef IMPORT_RADIOGROUP
    RadioGroups mxRadioGroups;
#endif

    inline ImportContext( LayoutRoot &rRoot ) SAL_THROW( () )
        : mrRoot( rRoot ) {}
    virtual ~ImportContext() {}

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xNamespaceMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & /* rTarget */, ::rtl::OUString const & /* rData */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & /* xLocator */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startRootElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

class ElementBase : public ::cppu::WeakImplHelper1< css::xml::input::XElement >
{
protected:
    ImportContext *mpImport;
/* TODO: check if all this memebers are needed. */
    ElementBase   *mpParent;
    sal_Int32      mnUid;

    ::rtl::OUString maLocalName;
    css::uno::Reference< css::xml::input::XAttributes > mxAttributes;
public:
    ElementBase(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, ImportContext * pImport )
    SAL_THROW( () );
    virtual ~ElementBase() SAL_THROW(());

    // XElement
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL getParent()
        throw (css::uno::RuntimeException)
    { return static_cast< css::xml::input::XElement * >( mpParent ); }
    virtual ::rtl::OUString SAL_CALL getLocalName() throw (css::uno::RuntimeException)
    { return maLocalName; }
    virtual sal_Int32 SAL_CALL getUid() throw (css::uno::RuntimeException)
    { return mnUid; }
    virtual css::uno::Reference< css::xml::input::XAttributes >
    SAL_CALL getAttributes() throw (css::uno::RuntimeException)
    { return mxAttributes; }

    virtual void SAL_CALL ignorableWhitespace(
        ::rtl::OUString const & /* rWhitespaces */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL characters( ::rtl::OUString const & /* rChars */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & /* Target */, ::rtl::OUString const & /* Data */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }

    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes > const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException) = 0;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException)
    { /* ignore */ }
};

class WidgetElement : public ElementBase
{
protected:
    LayoutWidget *mpWidget;

public:
    WidgetElement( sal_Int32 nUid, rtl::OUString const &name,
                   css::uno::Reference< css::xml::input::XAttributes > const &attributes,
                   ElementBase *parent, ImportContext *import ) SAL_THROW (());

    ~WidgetElement();


    virtual css::uno::Reference< css::xml::input::XElement> SAL_CALL
    startChildElement (sal_Int32 id, rtl::OUString const &name,
                       css::uno::Reference< css::xml::input::XAttributes > const &attributes)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException );
    virtual void SAL_CALL characters( ::rtl::OUString const & /* rChars */ )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

class ToplevelElement : public WidgetElement
{
public:
    ToplevelElement( rtl::OUString const &name,
                     css::uno::Reference< css::xml::input::XAttributes > const &attributes,
                     ImportContext *import ) SAL_THROW (());
    ~ToplevelElement();
};


} // namespace layoutimpl

#endif /* IMPORT_HXX */
