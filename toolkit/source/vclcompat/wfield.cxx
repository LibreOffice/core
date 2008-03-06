#include "wrapper.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XMetricField.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>

using namespace ::com::sun::star;

namespace layout
{

class EditImpl : public ControlImpl,
                 public ::cppu::WeakImplHelper1< awt::XTextListener >
{
    Link maModifyHdl;
  public:
    uno::Reference< awt::XTextComponent > mxEdit;
    EditImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow )
        ,  mxEdit( xPeer, uno::UNO_QUERY )
    {
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    {
        SetModifyHdl( Link() );
        mxEdit.clear();
    }

    void SetModifyHdl( const Link& rLink )
    {
        if ( !mxEdit.is() )
            return;
        maModifyHdl = rLink;
        if ( !rLink )
            mxEdit->removeTextListener( this );
        else
            mxEdit->addTextListener( this );
    }

    void SAL_CALL textChanged( const css::awt::TextEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        maModifyHdl.Call( mpWindow );
    }
};

void Edit::SetText( const XubString& rStr ) const
{
    if ( !getImpl().mxEdit.is() )
        return;
    getImpl().mxEdit->setText( rStr );
}

XubString Edit::GetText() const
{
    if ( !getImpl().mxEdit.is() )
        return XubString();
    return XubString( getImpl().mxEdit->getText());
}

void Edit::SetModifyHdl( const Link& rLink )
{
    getImpl().SetModifyHdl( rLink );
}

DECL_CONSTRUCTOR_IMPLS( Edit, Control, "edit" );
DECL_GET_IMPL_IMPL( Edit )

// Window/Control/Edit/MultiLineEdit
class MultiLineEditImpl : public EditImpl
{
public:
    MultiLineEditImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : EditImpl( pCtx, xPeer, pWindow )
    {
    }
};

DECL_CONSTRUCTOR_IMPLS( MultiLineEdit, Edit, "multilineedit" );
DECL_GET_IMPL_IMPL( MultiLineEdit )

// Window/Control/Edit/SpinField
class SpinFieldImpl : public EditImpl
{
  public:
    SpinFieldImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : EditImpl( pCtx, xPeer, pWindow ) {}
};

DECL_CONSTRUCTOR_IMPLS( SpinField, Edit, "spinfield" );

// Window/Control/Edit/SpinField/NumericField
class NumericFieldImpl : public SpinFieldImpl
{
  public:
    NumericFieldImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : SpinFieldImpl( pCtx, xPeer, pWindow ) {}
};

// Window/Control/Edit/SpinField/MetricField
class MetricFieldImpl : public SpinFieldImpl
{
  public:
    MetricFieldImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : SpinFieldImpl( pCtx, xPeer, pWindow ) {}
};

DECL_GET_IMPL_IMPL( SpinField )
DECL_GET_IMPL_IMPL( NumericField )
DECL_GET_IMPL_IMPL( MetricField )

// FormatterBase
class FormatterBaseImpl
{
  protected:
    PeerHandle mxPeer;
  public:
    explicit FormatterBaseImpl( const PeerHandle &xPeer )
        : mxPeer( xPeer ) {};
};

class NumericFormatterImpl : public FormatterBaseImpl
{
  public:
    uno::Reference< awt::XCurrencyField > mxField;
    explicit NumericFormatterImpl( const PeerHandle &xPeer )
        : FormatterBaseImpl( xPeer )
        , mxField( xPeer, uno::UNO_QUERY ) {}

    // FIXME: burn that CPU ! cut/paste from vclxwindows.cxx
    double valueToDouble( sal_Int64 nValue )
    {
        sal_Int16 nDigits = mxField->getDecimalDigits();
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n /= 10;
        return n;
    } // FIXME: burn that CPU ! cut/paste from vclxwindows.cxx
    sal_Int64 doubleToValue( double nValue )
    {
        sal_Int16 nDigits = mxField->getDecimalDigits();
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n *= 10;
        return (sal_Int64) n;
    }
};

class MetricFormatterImpl : public FormatterBaseImpl
{
  public:
    uno::Reference< awt::XMetricField > mxField;
    explicit MetricFormatterImpl( const PeerHandle &xPeer )
        : FormatterBaseImpl( xPeer )
        , mxField( xPeer, uno::UNO_QUERY ) {}
};

// NumericFormatter

NumericFormatter::NumericFormatter( FormatterBaseImpl *pImpl )
    : FormatterBase( pImpl )
{
}

NumericFormatterImpl& NumericFormatter::getFormatImpl() const
{
    return *( static_cast<NumericFormatterImpl *>( mpFormatImpl ) );
}

#define SET_IMPL(vclmethod, idlmethod) \
    void NumericFormatter::vclmethod( sal_Int64 nValue ) \
    { \
        if ( !getFormatImpl().mxField.is() ) \
            return; \
        getFormatImpl().mxField->idlmethod( getFormatImpl().valueToDouble( nValue ) ); \
    }

SET_IMPL( SetMin, setMin )
SET_IMPL( SetMax, setMax )
SET_IMPL( SetLast, setLast )
SET_IMPL( SetFirst, setFirst )
SET_IMPL( SetValue, setValue )
SET_IMPL( SetSpinSize, setSpinSize )

sal_Int64 NumericFormatter::GetValue() const
{
    if ( !getFormatImpl().mxField.is() )
        return 0;
    return getFormatImpl().doubleToValue( getFormatImpl().mxField->getValue() );
}

#undef SET_IMPL

DECL_CONSTRUCTOR_IMPLS_2( NumericField, SpinField, NumericFormatter, "numericfield" );

// MetricFormatter

MetricFormatter::MetricFormatter( FormatterBaseImpl *pImpl )
    : FormatterBase( pImpl ) {}
MetricFormatterImpl& MetricFormatter::getFormatImpl() const
{    return *( static_cast<MetricFormatterImpl *>( mpFormatImpl ) );   }

#define MetricUnitVclToUno(a) ((sal_uInt16)(a))

#define SET_IMPL(vclmethod, idlmethod) \
    void MetricFormatter::vclmethod( sal_Int64 nValue, FieldUnit nUnit ) \
    { \
        if ( !getFormatImpl().mxField.is() ) \
            return; \
        getFormatImpl().mxField->idlmethod( nValue, MetricUnitVclToUno( nUnit ) ); \
    }

SET_IMPL( SetMin, setMin )
SET_IMPL( SetMax, setMax )
SET_IMPL( SetLast, setLast )
SET_IMPL( SetFirst, setFirst )
SET_IMPL( SetValue, setValue )

#undef SET_IMPL

void MetricFormatter::SetSpinSize( sal_Int64 nValue )
{
    if ( !getFormatImpl().mxField.is() )
        return;
    getFormatImpl().mxField->setSpinSize( nValue );
}

sal_Int64 MetricFormatter::GetValue( FieldUnit nUnit ) const
{
    if ( !getFormatImpl().mxField.is() )
        return 0;
    return getFormatImpl().mxField->getValue( MetricUnitVclToUno( nUnit ) );
}

DECL_CONSTRUCTOR_IMPLS_2( MetricField, SpinField, MetricFormatter, "metricfield" );

// Window/Control/Edit/ComboBox
class ComboBoxImpl : public EditImpl,
                     public ::cppu::WeakImplHelper1< awt::XActionListener >,
                     public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maClickHdl, maSelectHdl;
public:
    uno::Reference< awt::XComboBox > mxComboBox;
    ComboBoxImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : EditImpl( pCtx, xPeer, pWindow ),
        mxComboBox( xPeer, uno::UNO_QUERY )
    {
    }

    USHORT InsertEntry(const XubString& rStr, USHORT nPos)
    {
        if ( nPos == COMBOBOX_APPEND )
            nPos = GetEntryCount();
        mxComboBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    void RemoveEntry( USHORT nPos )
    {
        mxComboBox->removeItems( nPos, 1 );
    }

    USHORT GetEntryPos( const XubString& rStr ) const
    {
        uno::Sequence< rtl::OUString> aItems( mxComboBox->getItems() );
        rtl::OUString rKey( rStr );
        for( unsigned int i = 0; aItems.getLength(); i++ )
        {
            if ( aItems[ i ] == rKey )
                return i;
        }
        return COMBOBOX_ENTRY_NOTFOUND;
    }

    XubString GetEntry( USHORT nPos ) const
    {
        return XubString( mxComboBox->getItem( nPos ) );
    }

    USHORT GetEntryCount() const
    {
        return mxComboBox->getItemCount();
    }

    void SetClickHdl( const Link& rLink )
    {
        maClickHdl = rLink;
        if ( !rLink )
            mxComboBox->removeActionListener( this );
        else
            mxComboBox->addActionListener( this );
    }

    void SetSelectHdl( const Link& rLink )
    {
        maSelectHdl = rLink;
        if ( !rLink )
            mxComboBox->removeItemListener( this );
        else
            mxComboBox->addItemListener( this );
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    {
        mxComboBox.clear();
    }

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        ComboBox* pComboBox = static_cast<ComboBox*>( mpWindow );
        if ( !pComboBox )
            return;
        maClickHdl.Call( pComboBox );
    }

    virtual void SAL_CALL itemStateChanged( const css::awt::ItemEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        ComboBox* pComboBox = static_cast<ComboBox*>( mpWindow );
        if ( !pComboBox )
            return;
        maSelectHdl.Call( pComboBox );
    }
};

USHORT ComboBox::InsertEntry( const XubString &rStr, USHORT nPos )
{
    return getImpl().InsertEntry( rStr, nPos );
}

void ComboBox::RemoveEntry( const XubString& rStr )
{
    getImpl().RemoveEntry( GetEntryPos( rStr ) );
}

void ComboBox::RemoveEntry( USHORT nPos )
{
    getImpl().RemoveEntry( nPos );
}

void ComboBox::Clear()
{
    uno::Sequence< rtl::OUString> aNoItems;
    getImpl().setProperty( "StringItemList", uno::Any( aNoItems ) );
}

USHORT ComboBox::GetEntryPos( const XubString& rStr ) const
{
    return getImpl().GetEntryPos( rStr );
}

XubString ComboBox::GetEntry( USHORT nPos ) const
{
    rtl::OUString rItem = getImpl().mxComboBox->getItem( nPos );
    return XubString( rItem );
}

USHORT ComboBox::GetEntryCount() const
{
    return getImpl().GetEntryCount();
}

void ComboBox::SetClickHdl( const Link& rLink )
{
    getImpl().SetClickHdl( rLink );
}

void ComboBox::SetSelectHdl( const Link& rLink )
{
    getImpl().SetSelectHdl( rLink );
}

DECL_CONSTRUCTOR_IMPLS( ComboBox, Edit, "combobox" );
DECL_GET_IMPL_IMPL( ComboBox )

// Window/Control/ListBox
class ListBoxImpl : public ControlImpl,
                    public ::cppu::WeakImplHelper1< awt::XActionListener >,
                    public ::cppu::WeakImplHelper1< awt::XItemListener >
{
    Link maClickHdl, maSelectHdl;
public:
    uno::Reference< awt::XListBox > mxListBox;
    ListBoxImpl( Context *pCtx, const PeerHandle &xPeer, Window *pWindow )
        : ControlImpl( pCtx, xPeer, pWindow ),
        mxListBox( xPeer, uno::UNO_QUERY )
    {
    }

    USHORT InsertEntry(const XubString& rStr, USHORT nPos)
    {
        if ( nPos == LISTBOX_APPEND )
            nPos = mxListBox->getItemCount();
        mxListBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    void RemoveEntry( USHORT nPos )
    {
        mxListBox->removeItems( nPos, 1 );
    }

    USHORT RemoveEntry(const XubString& rStr, USHORT nPos)
    {
        if ( nPos == LISTBOX_APPEND )
            nPos = mxListBox->getItemCount();
        mxListBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    USHORT GetEntryPos( const XubString& rStr ) const
    {
        uno::Sequence< rtl::OUString> aItems( mxListBox->getItems() );
        rtl::OUString rKey( rStr );
        for( unsigned int i = 0; aItems.getLength(); i++ )
        {
            if ( aItems[ i ] == rKey )
                return i;
        }
        return LISTBOX_ENTRY_NOTFOUND;
    }

    XubString GetEntry( USHORT nPos ) const
    {
        return mxListBox->getItem( nPos );
    }

    USHORT GetEntryCount() const
    {
        return mxListBox->getItemCount();
    }

    void SelectEntryPos( USHORT nPos, BOOL bSelect )
    {
        mxListBox->selectItemPos( nPos, bSelect );
    }

    USHORT GetSelectEntryCount() const
    {
        return mxListBox->getSelectedItems().getLength();
    }

    USHORT GetSelectEntryPos( USHORT nSelIndex ) const
    {
        USHORT nSelected = 0;
        if ( mxListBox->isMutipleMode() )
        {
            uno::Sequence< short > aItems( mxListBox->getSelectedItemsPos() );
            if ( nSelIndex < aItems.getLength() )
                nSelected = aItems[ nSelIndex ];
        }
        else
            nSelected = mxListBox->getSelectedItemPos();
        return nSelected;
    }

    void SetClickHdl( const Link& rLink )
    {
        maClickHdl = rLink;
        if ( !rLink )
            mxListBox->removeActionListener( this );
        else
            mxListBox->addActionListener( this );
    }

    void SetSelectHdl( const Link& rLink )
    {
        maSelectHdl = rLink;
        if ( !rLink )
            mxListBox->removeItemListener( this );
        else
            mxListBox->addItemListener( this );
    }

    virtual void SAL_CALL disposing( const css::lang::EventObject& /* Source */ )
        throw (css::uno::RuntimeException)
    {
        mxListBox.clear();
    }

    void SAL_CALL actionPerformed( const css::awt::ActionEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        maClickHdl.Call( mpWindow );
    }

    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& /* rEvent */ )
        throw (css::uno::RuntimeException)
    {
        ListBox* pListBox = static_cast<ListBox*>( mpWindow );
        if ( !pListBox )
            return;
        maSelectHdl.Call( pListBox );
    }
};

USHORT ListBox::InsertEntry(const XubString& rStr, USHORT nPos)
{
    return getImpl().InsertEntry(rStr, nPos);
}

void ListBox::RemoveEntry( USHORT nPos )
{
    return getImpl().RemoveEntry( nPos );
}

void ListBox::RemoveEntry( const XubString& rStr )
{
    return getImpl().RemoveEntry( GetEntryPos( rStr ) );
}

void ListBox::Clear()
{
    uno::Sequence< rtl::OUString> aNoItems;
    getImpl().setProperty( "StringItemList", uno::Any( aNoItems ) );
}

USHORT ListBox::GetEntryPos( const XubString& rStr ) const
{
    return getImpl().GetEntryPos( rStr );
}

XubString ListBox::GetEntry( USHORT nPos ) const
{
    return getImpl().GetEntry( nPos );
}

USHORT ListBox::GetEntryCount() const
{
    return getImpl().GetEntryCount();
}

void ListBox::SelectEntryPos( USHORT nPos, BOOL bSelect )
{
    getImpl().SelectEntryPos( nPos, bSelect );
}

void ListBox::SelectEntry( const XubString& rStr, BOOL bSelect )
{
    SelectEntryPos( GetEntryPos( rStr ), bSelect );
}

USHORT ListBox::GetSelectEntryCount() const
{
    return getImpl().GetSelectEntryCount();
}

USHORT ListBox::GetSelectEntryPos( USHORT nSelIndex ) const
{
    return getImpl().GetSelectEntryPos( nSelIndex );
}

XubString ListBox::GetSelectEntry( USHORT nSelIndex ) const
{
    return GetEntry( GetSelectEntryPos( nSelIndex ) );
}

void ListBox::SetSelectHdl( const Link& rLink )
{
    getImpl().SetSelectHdl( rLink );
}

void ListBox::SetClickHdl( const Link& rLink )
{
    getImpl().SetClickHdl( rLink );
}

DECL_CONSTRUCTOR_IMPLS( ListBox, Control, "listbox" );
DECL_GET_IMPL_IMPL( ListBox )

}; // end namespace layout
