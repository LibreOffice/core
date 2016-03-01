#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_CATEGORYLISTBOX_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_CATEGORYLISTBOX_HXX

#include <vcl/lstbox.hxx>
#include <vcl/builderfactory.hxx>

namespace sd {

class CategoryListBox : public ListBox
{
public:
    explicit CategoryListBox( vcl::Window* pParent );
    virtual ~CategoryListBox();

    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;

    sal_Int32           InsertCategory( const OUString& rStr );

    DECL_LINK_TYPED(implDoubleClickHdl, ListBox&, void);

private:
    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;

    Link<CategoryListBox&,void>     maDoubleClickHdl;
};

}

#endif
