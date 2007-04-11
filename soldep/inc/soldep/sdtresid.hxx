#ifndef _SOLDEV_RESID_HXX_
#define _SOLDEV_RESID_HXX_
#include <tools/resmgr.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>


class SolDevDll
{
    ResMgr*             pResMgr;
public:
                        SolDevDll();
                        ~SolDevDll();

    ResMgr*             GetResMgr() { return pResMgr; }
};

class DtSodResId : public ResId
{
public:
    DtSodResId(USHORT nId);
};

#endif

