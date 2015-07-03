#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRLINESH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRLINESH_HXX

#include "drwbassh.hxx"

class SwDrawBaseShell;

class SwDrawLineShell: public SwDrawBaseShell
{
public:
    SFX_DECL_INTERFACE(SW_DRAWLINESHELL)
    TYPEINFO_OVERRIDE();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                SwDrawLineShell(SwView &rView);

    void        ExecDrawDlg(SfxRequest& rReq);
    void        ExecDrawAttrArgs(SfxRequest& rReq);
    void        GetDrawAttrState(SfxItemSet &rSet);
};

#endif

