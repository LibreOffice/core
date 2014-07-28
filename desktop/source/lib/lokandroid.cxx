/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jni.h>

#include <sal/types.h>

#include <android/log.h>

#include <osl/detail/android-bootstrap.h>

#define LOK_USE_UNSTABLE_API

#include <LibreOfficeKit/LibreOfficeKit.h>

/* LibreOfficeKit */

jfieldID getHandleField(JNIEnv* pEnv, jobject aObject)
{
    jclass clazz = pEnv->GetObjectClass(aObject);
    return pEnv->GetFieldID(clazz, "handle", "J");
}

template <typename T>
T* getHandle(JNIEnv* pEnv, jobject aObject)
{
    jlong aHandle = pEnv->GetLongField(aObject, getHandleField(pEnv, aObject));
    return reinterpret_cast<T*>(aHandle);
}

template <typename T>
void setHandle(JNIEnv* pEnv, jobject aObject, T* aType)
{
    jlong aHandle = reinterpret_cast<jlong>(aType);
    pEnv->SetLongField(aObject, getHandleField(pEnv, aObject), aHandle);
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Office_getError(JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);
    char* pError = pLibreOfficeKit->pClass->getError(pLibreOfficeKit);
    return pEnv->NewStringUTF(pError);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_initialize(JNIEnv* pEnv, jobject aObject, jlong aLokHandle)
{
    pEnv->SetLongField(aObject, getHandleField(pEnv, aObject), aLokHandle);
}

extern "C" SAL_JNI_EXPORT jlong JNICALL Java_org_libreoffice_kit_Office_documentLoadNative(JNIEnv* pEnv, jobject aObject, jstring documentPath)
{
    const char* aCloneDocumentPath;

    const char* aCharDocumentPath = pEnv->GetStringUTFChars(documentPath, NULL);
    aCloneDocumentPath = strdup(aCharDocumentPath);
    pEnv->ReleaseStringUTFChars(documentPath, aCharDocumentPath);

    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);

    LibreOfficeKitDocument* pDocument = pLibreOfficeKit->pClass->documentLoad(pLibreOfficeKit, aCloneDocumentPath);
    return (jlong) (intptr_t) pDocument;
}

/* Document */
extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setPart
    (JNIEnv* pEnv, jobject aObject, jint aPart)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->setPart(pDocument, aPart);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getPart
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    return (jint) pDocument->pClass->getPart(pDocument);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getParts
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    return (jint) pDocument->pClass->getParts(pDocument);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getDocumentTypeNative
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    return (jint) pDocument->pClass->getDocumentType(pDocument);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_paintTileNative
    (JNIEnv* pEnv, jobject aObject, jobject aByteBuffer,
    jint nCanvasWidth, jint nCanvasHeight, jint nTilePosX, jint nTilePosY,
    jint nTileWidth, jint nTileHeight)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);

    unsigned char* buffer = (unsigned char*) pEnv->GetDirectBufferAddress(aByteBuffer);
    int nStride = 0;
    pDocument->pClass->paintTile(pDocument, buffer, nCanvasWidth, nCanvasHeight, &nStride, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    (void) nStride;
}

extern "C" SAL_JNI_EXPORT jlong JNICALL Java_org_libreoffice_kit_Document_getDocumentHeight
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    long nWidth;
    long nHeight;
    pDocument->pClass->getDocumentSize(pDocument, &nWidth, &nHeight);
    return nHeight;
}

extern "C" SAL_JNI_EXPORT jlong JNICALL Java_org_libreoffice_kit_Document_getDocumentWidth
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    long nWidth;
    long nHeight;
    pDocument->pClass->getDocumentSize(pDocument, &nWidth, &nHeight);
    return nWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
