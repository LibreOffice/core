#include "iipaobj.hxx"
#include "embeddoc.hxx"



CIIAObj::CIIAObj(EmbedDocument_Impl *pEmbDoc,DocumentHolder* pDocHolder)
    : m_cRef(0),
      m_pEmbDoc(pEmbDoc),
      m_pDocHolder(pDocHolder)
{
}


CIIAObj::~CIIAObj()
{
    return;
}

/* IUnknown methods */

STDMETHODIMP CIIAObj::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv=NULL;

    if(IID_IUnknown==riid ||
       IID_IOleWindow==riid ||
       IID_IOleInPlaceActiveObject==riid)
        *ppv=this;

    //AddRef any interface we'll return.
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CIIAObj::AddRef(void)
{
    ++m_cRef;
    return m_pEmbDoc->AddRef();
}

STDMETHODIMP_(ULONG) CIIAObj::Release(void)
{
    --m_cRef;
    return m_pEmbDoc->Release();
}



/* IOleInPlaceActiveObject methods*/

STDMETHODIMP CIIAObj::GetWindow(HWND *)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::ContextSensitiveHelp(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::TranslateAccelerator(LPMSG)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::OnFrameWindowActivate(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::OnDocWindowActivate(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::ResizeBorder(
    LPCRECT pRect,LPOLEINPLACEUIWINDOW,BOOL bFrame)
{
    if(!bFrame) return NOERROR;

    return m_pDocHolder->SetContRects(pRect);
}


STDMETHODIMP CIIAObj::EnableModeless(BOOL)
{
    return NOERROR;
}
