/*************************************************************************
 *
 *  $RCSfile: errinf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <limits.h>
#include <shl.hxx>
#include <debug.hxx>
#include <errinf.hxx>
#include <string.hxx>

class ErrorHandler;

struct EDcrData
{
  public:

    ErrorHandler               *pFirstHdl;
    ErrorContext               *pFirstCtx;
    void                       *pDsp;
    BOOL                       bIsWindowDsp;


    DynamicErrorInfo            *ppDcr[ERRCODE_DYNAMIC_COUNT];
    USHORT                      nNextDcr;
                                EDcrData();

static  EDcrData                *GetData();

};

class EDcr_Impl
{
    ULONG                       lErrId;
    USHORT                      nMask;

    void                        RegisterEDcr(DynamicErrorInfo *);
    void                        UnRegisterEDcr(DynamicErrorInfo *);
    static ErrorInfo           *GetDynamicErrorInfo(ULONG lId);

friend class DynamicErrorInfo;
friend class ErrorInfo;
};


EDcrData::EDcrData()
{
    for(USHORT n=0;n<ERRCODE_DYNAMIC_COUNT;n++)
        ppDcr[n]=0;
    nNextDcr=0;
    pFirstHdl=0;
    pDsp=0;
    pFirstCtx=0;
}


EDcrData *EDcrData::GetData()
{
#ifdef BOOTSTRAP
    return 0x0;
#else
    EDcrData **ppDat=(EDcrData **)GetAppData(SHL_ERR);
    if(!*ppDat)
    {
        return (*ppDat=new EDcrData);
    }
    else
        return *ppDat;
#endif
}

void EDcr_Impl::RegisterEDcr(DynamicErrorInfo *pDcr)
{
    //Vergibt eine dynamische Id

    EDcrData* pData=EDcrData::GetData();
    lErrId= (((ULONG)pData->nNextDcr + 1) << ERRCODE_DYNAMIC_SHIFT) +
        pDcr->GetErrorCode();
    DynamicErrorInfo **ppDcr=pData->ppDcr;
    USHORT nNext=pData->nNextDcr;

    // bei einem Ringbuffer koennen wir uns das ASSERT wohl sparen!
    // DBG_ASSERT(ppDcr[nNext]==0,"ErrHdl: Alle Errors belegt");
    if(ppDcr[nNext])
    {
        delete ppDcr[nNext];
    }
    ppDcr[nNext]=pDcr;
    if(++pData->nNextDcr>=ERRCODE_DYNAMIC_COUNT)
        pData->nNextDcr=0;
}


void EDcr_Impl::UnRegisterEDcr(DynamicErrorInfo *pDcr)
{

    EDcrData* pData=EDcrData::GetData();
    DynamicErrorInfo **ppDcr=pData->ppDcr;
    ULONG lIdx=(
        ((ULONG)(*pDcr) & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DBG_ASSERT(ppDcr[lIdx]==pDcr,"ErrHdl: Error nicht gefunden");
    if(ppDcr[lIdx]==pDcr)
        ppDcr[lIdx]=0;
}

TYPEINIT0(ErrorInfo);
TYPEINIT1(DynamicErrorInfo, ErrorInfo);
TYPEINIT1(StandardErrorInfo, DynamicErrorInfo);
TYPEINIT1(StringErrorInfo, DynamicErrorInfo);
TYPEINIT1(TwoStringErrorInfo, DynamicErrorInfo);
TYPEINIT1(MessageInfo, DynamicErrorInfo);


ErrorInfo *ErrorInfo::GetErrorInfo(ULONG lId)
{
    if(lId & ERRCODE_DYNAMIC_MASK)
        return EDcr_Impl::GetDynamicErrorInfo(lId);
    else
        return new ErrorInfo(lId);
}

DynamicErrorInfo::operator ULONG() const
{
    return pImpl->lErrId;
}

DynamicErrorInfo::DynamicErrorInfo(ULONG lArgUserId, USHORT nMask)
: ErrorInfo(lArgUserId)
{
    pImpl=new EDcr_Impl;
    pImpl->RegisterEDcr(this);
    pImpl->nMask=nMask;
}

DynamicErrorInfo::~DynamicErrorInfo()
{
    pImpl->UnRegisterEDcr(this);
    delete pImpl;
}

ErrorInfo* EDcr_Impl::GetDynamicErrorInfo(ULONG lId)
{
    ULONG lIdx=((lId & ERRCODE_DYNAMIC_MASK)>>ERRCODE_DYNAMIC_SHIFT)-1;
    DynamicErrorInfo* pDcr=EDcrData::GetData()->ppDcr[lIdx];
    if(pDcr && (ULONG)(*pDcr)==lId)
        return pDcr;
    else
        return new ErrorInfo(lId & ~ERRCODE_DYNAMIC_MASK);
}


USHORT DynamicErrorInfo::GetDialogMask() const
{
    return pImpl->nMask;
}


StandardErrorInfo::StandardErrorInfo(
    ULONG lUserId, ULONG lArgExtId, USHORT nFlags)
: DynamicErrorInfo(lUserId, nFlags), lExtId(lArgExtId)
{
}


StringErrorInfo::StringErrorInfo(
    ULONG lUserId, const String& aStringP,  USHORT nFlags)
: DynamicErrorInfo(lUserId, nFlags), aString(aStringP)
{
}


class ErrHdl_Impl
{
  public:

    ErrorHandler        *pNext;
    static BOOL         CreateString(const ErrorHandler *pStart,
                                     const ErrorInfo*, String&, USHORT&);
};


static void aDspFunc(const String &rErr, const String &rAction)
{
    ByteString aErr("Aktion: ");
    aErr+= ByteString( rAction, RTL_TEXTENCODING_ASCII_US );
    aErr+=" Fehler: ";
    aErr+= ByteString( rErr, RTL_TEXTENCODING_ASCII_US );
    DBG_ERROR(aErr.GetBuffer());
}


ErrorContext::ErrorContext(Window *pWinP)
{
    EDcrData *pData=EDcrData::GetData();
    ErrorContext *&pHdl=pData->pFirstCtx;
    pWin=pWinP;
    pNext=pHdl;
    pHdl=this;
}

ErrorContext::~ErrorContext()
{
    ErrorContext **ppCtx=&(EDcrData::GetData()->pFirstCtx);
    while(*ppCtx && *ppCtx!=this)
        ppCtx=&((*ppCtx)->pNext);
    if(*ppCtx)
        *ppCtx=(*ppCtx)->pNext;
}

ErrorContext *ErrorContext::GetContext()
{
    return EDcrData::GetData()->pFirstCtx;
}

ErrorHandler::ErrorHandler()
{
    pImpl=new ErrHdl_Impl;
    EDcrData *pData=EDcrData::GetData();
    ErrorHandler *&pHdl=pData->pFirstHdl;
    pImpl->pNext=pHdl;
    pHdl=this;
    if(!pData->pDsp)
        RegisterDisplay(&aDspFunc);
}

ErrorHandler::~ErrorHandler()
{
    ErrorHandler **ppHdl=&(EDcrData::GetData()->pFirstHdl);
    while(*ppHdl && *ppHdl!=this)
        ppHdl=&((*ppHdl)->pImpl->pNext);
    if(*ppHdl)
        *ppHdl=(*ppHdl)->pImpl->pNext;
    delete pImpl;
}

void ErrorHandler::RegisterDisplay(WindowDisplayErrorFunc *aDsp)
{
    EDcrData *pData=EDcrData::GetData();
    pData->bIsWindowDsp=TRUE;
    pData->pDsp=(void*)aDsp;
}

void ErrorHandler::RegisterDisplay(BasicDisplayErrorFunc *aDsp)
{
    EDcrData *pData=EDcrData::GetData();
    pData->bIsWindowDsp=FALSE;
    pData->pDsp=(void*)aDsp;
}

USHORT ErrorHandler::HandleError(ULONG lId, USHORT nFlags)
{

/*  [Beschreibung]
    Handelt einen Fehler ab. lId ist die FehlerId, nFlags sind die
    ErrorFlags. Werden nFlags nicht abgegeben, so werden die in
    der DynamicErrorInfo angegebenen Flags bzw. die aus der Resource
    verwendet.

    Also:

    1. nFlags,
    2. Resource Flags
    3. Dynamic Flags
    4. Default ERRCODE_BUTTON_OK, ERRCODE_MSG_ERROR


    */

    String aErr;
    String aAction;
    if(!lId || lId == ERRCODE_ABORT)
        return 0;
    EDcrData *pData=EDcrData::GetData();
    ErrorInfo *pInfo=ErrorInfo::GetErrorInfo(lId);
    ErrorContext *pCtx=ErrorContext::GetContext();
    if(pCtx)
        pCtx->GetString(pInfo->GetErrorCode(), aAction);
    Window *pParent=0;
    //Nimm den Parent aus dem Konext
    for(;pCtx;pCtx=pCtx->pNext)
        if(pCtx->GetParent())
        {
            pParent=pCtx->GetParent();
            break;
        }

    USHORT nErrFlags = ERRCODE_BUTTON_DEF_OK | ERRCODE_BUTTON_OK |
        ERRCODE_MSG_ERROR;

    DynamicErrorInfo* pDynPtr=PTR_CAST(DynamicErrorInfo,pInfo);
    if(pDynPtr)
    {
        USHORT nDynFlags = pDynPtr->GetDialogMask();
        if( nDynFlags )
            nErrFlags = nDynFlags;
    }

    if(ErrHdl_Impl::CreateString(pData->pFirstHdl,pInfo,aErr,nErrFlags))
    {
        delete pInfo;
        if(!pData->pDsp)
        {

            ByteString aStr("Action: ");
            aStr += ByteString( aAction, RTL_TEXTENCODING_ASCII_US );
            aStr += ByteString("\nFehler: ");
            aStr += ByteString( aErr, RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aStr.GetBuffer() );
        }
        else
            if(!pData->bIsWindowDsp)
            {
                (*(BasicDisplayErrorFunc*)pData->pDsp)(aErr,aAction);
                return 0;
            }
            else
            {
                if( nFlags != USHRT_MAX )
                    nErrFlags = nFlags;
                return (*(WindowDisplayErrorFunc*)pData->pDsp)(
                    pParent, nErrFlags, aErr, aAction);
            }
    }

    DBG_ERROR("Error nicht behandelt");
    // Error 1 ist General Error im Sfx
    if(pInfo->GetErrorCode()!=1)
        HandleError(1, USHRT_MAX);
    else
        DBG_ERROR("Error 1 nicht gehandeled");
    delete pInfo;
    return 0;
}

BOOL ErrorHandler::ForwCreateString(const ErrorInfo* pInfo, String& rStr, USHORT &rFlags) const
{
    return ErrHdl_Impl::CreateString(this->pImpl->pNext, pInfo, rStr, rFlags);
}

BOOL ErrHdl_Impl::CreateString( const ErrorHandler *pStart,
                                const ErrorInfo* pInfo, String& pStr,
                               USHORT &rFlags)
{
    for(const ErrorHandler *pHdl=pStart;pHdl;pHdl=pHdl->pImpl->pNext)
    {
        if(pHdl->CreateString( pInfo, pStr, rFlags))
            return TRUE;
    }
    return FALSE;
}

BOOL SimpleErrorHandler::CreateString(
    const ErrorInfo *pInfo, String &rStr, USHORT &) const
{
    ULONG nId = pInfo->GetErrorCode();
    ByteString aStr;
    aStr="Id ";
    aStr+=ByteString::CreateFromInt32(nId);
    aStr+=" only handled by SimpleErrorHandler";
    aStr+="\nErrorCode: ";
    aStr+=ByteString::CreateFromInt32(nId & ((1L <<  ERRCODE_CLASS_SHIFT)  - 1 ));
    aStr+="\nErrorClass: ";
    aStr+=ByteString::CreateFromInt32((nId & ERRCODE_CLASS_MASK) >> ERRCODE_CLASS_SHIFT);
    aStr+="\nErrorArea: ";
    aStr+=ByteString::CreateFromInt32((nId & ERRCODE_ERROR_MASK &
            ~((1 << ERRCODE_AREA_SHIFT ) -1 ) ) >> ERRCODE_AREA_SHIFT);
    DynamicErrorInfo *pDyn=PTR_CAST(DynamicErrorInfo,pInfo);
    if(pDyn)
    {
        aStr+="\nDId ";
        aStr+=ByteString::CreateFromInt32((ULONG)*pDyn);
    }
    StandardErrorInfo *pStd=PTR_CAST(StandardErrorInfo,pInfo);
    if(pStd)
    {
        aStr+="\nXId ";
        aStr+=ByteString::CreateFromInt32(pStd->GetExtendedErrorCode());
    }
    rStr = String( aStr, RTL_TEXTENCODING_ASCII_US );
    return TRUE;
}

SimpleErrorHandler::SimpleErrorHandler()
 : ErrorHandler()
{
}

