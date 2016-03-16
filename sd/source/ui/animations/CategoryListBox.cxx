#include "CategoryListBox.hxx"

namespace sd {

CategoryListBox::CategoryListBox( vcl::Window* pParent )
: ListBox( pParent, WB_TABSTOP | WB_BORDER )
{
    EnableUserDraw( true );
    SetDoubleClickHdl( LINK( this, CategoryListBox, implDoubleClickHdl ) );
}

VCL_BUILDER_FACTORY(CategoryListBox)

CategoryListBox::~CategoryListBox()
{
}

sal_Int32  CategoryListBox::InsertCategory( const OUString& rStr )
{
    sal_Int32  n = ListBox::InsertEntry( rStr );
    if( n != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SetEntryFlags( n, ListBox::GetEntryFlags(n) | ListBoxEntryFlags::DisableSelection );

    return n;
}

void CategoryListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    const sal_uInt16 nItem = rUDEvt.GetItemId();

    if( ListBox::GetEntryFlags(nItem) & ListBoxEntryFlags::DisableSelection )
    {
        Rectangle aOutRect( rUDEvt.GetRect() );
        vcl::RenderContext* pDev = rUDEvt.GetRenderContext();

        // fill the background
        Color aColor (GetSettings().GetStyleSettings().GetDialogColor());

        pDev->SetFillColor (aColor);
        pDev->SetLineColor ();
        pDev->DrawRect(aOutRect);

        // Erase the four corner pixels to make the rectangle appear rounded.
        pDev->SetLineColor( GetSettings().GetStyleSettings().GetWindowColor());
        pDev->DrawPixel( aOutRect.TopLeft());
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Top()));
        pDev->DrawPixel( Point(aOutRect.Left(), aOutRect.Bottom()));
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Bottom()));

        // draw the category title
        pDev->DrawText (aOutRect, GetEntry(nItem), DrawTextFlags::Center );
    }
    else
    {
        DrawEntry( rUDEvt, true, false );
    }
}

IMPL_LINK_NOARG_TYPED(CategoryListBox, implDoubleClickHdl, ListBox&, void)
{
    CaptureMouse();
}

void CategoryListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();
    if( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) )
    {
        maDoubleClickHdl.Call( *this );
    }
    else
    {
        ListBox::MouseButtonUp( rMEvt );
    }
}

}
