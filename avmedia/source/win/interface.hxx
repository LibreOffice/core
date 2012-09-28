/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        BYTE *pBuffer,
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
        BOOL OneShot) = 0;
    virtual HRESULT __stdcall SetMediaType(
        const AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall GetConnectedMediaType(
        AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall SetBufferSamples(
        BOOL BufferThem) = 0;
    virtual HRESULT __stdcall GetCurrentBuffer(
         long *pBufferSize,
         long *pBuffer) = 0;
    virtual HRESULT __stdcall GetCurrentSample(
         IMediaSample **ppSample) = 0;
    virtual HRESULT __stdcall SetCallback(
        ISampleGrabberCB *pCallback,
        long WhichMethodToCallback) = 0;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
