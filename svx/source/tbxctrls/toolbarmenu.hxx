#include <vcl/ctrl.hxx>

#include <vector>

class ToolbarMenuEntry;
typedef std::vector< ToolbarMenuEntry * > ToolbarMenuEntryVector;

class ToolbarMenu : public Control
{
private:
    ToolbarMenuEntryVector  maEntryVector;

    int mnCheckPos;
    int mnImagePos;
    int mnTextPos;

    int mnHighlightedEntry;
    int mnSelectedEntry;

    Size maSize;

    Link            maHighlightHdl;
    Link            maSelectHdl;

    void            StateChanged( StateChangedType nType );
    void            DataChanged( const DataChangedEvent& rDCEvt );

    void            initWindow();

    Size            implCalcSize();

    void            appendEntry( ToolbarMenuEntry* pEntry );

    void            implPaint( ToolbarMenuEntry* pThisOnly = NULL, bool bHighlight = false );

    void            implHighlightEntry( int nHighlightEntry, bool bHighlight );
    void            implHighlightEntry( const MouseEvent& rMEvt, bool bMBDown );

    void            implChangeHighlightEntry( int nEntry );
    void            implSelectEntry( int nSelectedEntry );

    ToolbarMenuEntry*   implCursorUpDown( bool bUp, bool bHomeEnd );
    ToolbarMenuEntry*   implGetEntry( int nEntry ) const;
    ToolbarMenuEntry*   implSearchEntry( int nEntryId ) const;

public:
                    ToolbarMenu( Window* pParent, WinBits nStyle );
                    ~ToolbarMenu();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();

    void            appendEntry( int nEntryId, const String& rStr, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, const Image& rImage, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, const String& rStr, const Image& rImage, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, Control* pControl, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, const String& rStr, Control* pControl, MenuItemBits nItemBits = 0 );
    void            appendSeparator();

    void            checkEntry( int nEntryId, bool bCheck = true );
    bool            isEntryChecked( int nEntryId ) const;

    void            enableEntry( int nEntryId, bool bEnable = true );
    bool            isEntryEnabled( int nEntryId ) const;

    void            setEntryText( int nEntryId, const String& rStr );
    const String&   getEntryText( int nEntryId ) const;

    void            setEntryImage( int nEntryId, const Image& rImage );
    const Image&    getEntryImage( int nEntryId ) const;

    const Size&     getMenuSize() const { return maSize; }

    void            SetHighlightHdl( const Link& rLink )    { maHighlightHdl = rLink; }
    const Link&     GetHighlightHdl() const                 { return maHighlightHdl; }

    void            SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                    { return maSelectHdl; }

    int             getSelectedEntryId() const;
    int             getHighlightedEntryId() const;
};
