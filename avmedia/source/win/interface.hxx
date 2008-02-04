/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 12:56:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

extern "C" const CLSID CLSID_MediaDet;
extern "C" const IID IID_IMediaDet;
struct ISampleGrabber;

struct
#ifndef __MINGW32__
__declspec(uuid("65BD0710-24D2-4ff7-9324-ED2E5D3ABAFA")) __declspec(novtable)
#endif
IMediaDet : public IUnknown
{
public:
    virtual  HRESULT __stdcall get_Filter(
          IUnknown **pVal) = 0;
    virtual  HRESULT __stdcall put_Filter(
          IUnknown *newVal) = 0;
    virtual  HRESULT __stdcall get_OutputStreams(
          long *pVal) = 0;
    virtual  HRESULT __stdcall get_CurrentStream(
          long *pVal) = 0;
    virtual  HRESULT __stdcall put_CurrentStream(
         long newVal) = 0;
    virtual  HRESULT __stdcall get_StreamType(
          GUID *pVal) = 0;
    virtual  HRESULT __stdcall get_StreamTypeB(
          BSTR *pVal) = 0;
    virtual  HRESULT __stdcall get_StreamLength(
          double *pVal) = 0;
    virtual  HRESULT __stdcall get_Filename(
          BSTR *pVal) = 0;
    virtual  HRESULT __stdcall put_Filename(
          BSTR newVal) = 0;
    virtual  HRESULT __stdcall GetBitmapBits(
        double StreamTime,
         long *pBufferSize,
         char *pBuffer,
        long Width,
        long Height) = 0;
    virtual  HRESULT __stdcall WriteBitmapBits(
        double StreamTime,
        long Width,
        long Height,
         BSTR Filename) = 0;
    virtual  HRESULT __stdcall get_StreamMediaType(
          AM_MEDIA_TYPE *pVal) = 0;
    virtual  HRESULT __stdcall GetSampleGrabber(
          ISampleGrabber **ppVal) = 0;
    virtual  HRESULT __stdcall get_FrameRate(
          double *pVal) = 0;
    virtual  HRESULT __stdcall EnterBitmapGrabMode(
        double SeekTime) = 0;
};

extern "C" const IID IID_ISampleGrabberCB;
struct
#ifndef __MINGW32__
__declspec(uuid("0579154A-2B53-4994-B0D0-E773148EFF85")) __declspec(novtable)
#endif
ISampleGrabberCB : public IUnknown
{
public:
    virtual HRESULT __stdcall SampleCB(
        double SampleTime,
        IMediaSample *pSample) = 0;
    virtual HRESULT __stdcall BufferCB(
        double SampleTime,
        WIN_BYTE *pBuffer,
        long BufferLen) = 0;
};

extern "C" const IID IID_ISampleGrabber;
struct
#ifndef __MINGW32__
__declspec(uuid("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")) __declspec(novtable)
#endif
ISampleGrabber : public IUnknown
{
public:
    virtual HRESULT __stdcall SetOneShot(
        WIN_BOOL OneShot) = 0;
    virtual HRESULT __stdcall SetMediaType(
        const AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall GetConnectedMediaType(
        AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall SetBufferSamples(
        WIN_BOOL BufferThem) = 0;
    virtual HRESULT __stdcall GetCurrentBuffer(
         long *pBufferSize,
         long *pBuffer) = 0;
    virtual HRESULT __stdcall GetCurrentSample(
         IMediaSample **ppSample) = 0;
    virtual HRESULT __stdcall SetCallback(
        ISampleGrabberCB *pCallback,
        long WhichMethodToCallback) = 0;

};

