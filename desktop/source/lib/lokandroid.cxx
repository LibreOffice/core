/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unistd.h>
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
    return pEnv->GetFieldID(clazz, "handle", "Ljava/nio/ByteBuffer;");
}

template <typename T>
T* getHandle(JNIEnv* pEnv, jobject aObject)
{
    jobject aHandle = pEnv->GetObjectField(aObject, getHandleField(pEnv, aObject));
    return reinterpret_cast<T*>(pEnv->GetDirectBufferAddress(aHandle));
}

const char* copyJavaString(JNIEnv* pEnv, jstring aJavaString)
{
    const char* pClone = NULL;

    const char* pTemp = pEnv->GetStringUTFChars(aJavaString, NULL);
    pClone = strdup(pTemp);
    pEnv->ReleaseStringUTFChars(aJavaString, pTemp);

    return pClone;
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Office_getError(JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);
    char* pError = pLibreOfficeKit->pClass->getError(pLibreOfficeKit);
    return pEnv->NewStringUTF(pError);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_destroy(JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);
    pLibreOfficeKit->pClass->destroy(pLibreOfficeKit);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_destroyAndExit(JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);
    pLibreOfficeKit->pClass->destroy(pLibreOfficeKit);
    // Stopgap fix: _exit() to force the OS to restart the LO activity.
    // Better than to hang.
    _exit(0);
}

extern "C" SAL_JNI_EXPORT jobject JNICALL Java_org_libreoffice_kit_Office_documentLoadNative(JNIEnv* pEnv, jobject aObject, jstring documentPath)
{
    const char* aCloneDocumentPath = copyJavaString(pEnv, documentPath);
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);

    LibreOfficeKitDocument* pDocument = pLibreOfficeKit->pClass->documentLoad(pLibreOfficeKit, aCloneDocumentPath);
    jobject aHandle = pEnv->NewDirectByteBuffer((void*) pDocument, sizeof(LibreOfficeKitDocument));

    return aHandle;
}

/* Document */

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_destroy
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->destroy(pDocument);
}

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

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Document_getPartName
    (JNIEnv* pEnv, jobject aObject, jint nPartIndex)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    char* pPartName = pDocument->pClass->getPartName(pDocument, nPartIndex);
    return pEnv->NewStringUTF(pPartName);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setPartMode
    (JNIEnv* pEnv, jobject aObject, jint nPartMode)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);

    pDocument->pClass->setPartMode(pDocument, (LibreOfficeKitPartMode) nPartMode);
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

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_initializeForRendering
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->initializeForRendering(pDocument);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Office_saveAs
    (JNIEnv* pEnv, jobject aObject, jstring sUrl, jstring sFormat, jstring sOptions)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);

    const char* pUrl = pEnv->GetStringUTFChars(sUrl, NULL);
    const char* pFormat = pEnv->GetStringUTFChars(sFormat, NULL);
    const char* pOptions = pEnv->GetStringUTFChars(sOptions, NULL);

    int result = pDocument->pClass->saveAs(pDocument, pUrl, pFormat, pOptions);

    pEnv->ReleaseStringUTFChars(sUrl, pUrl);
    pEnv->ReleaseStringUTFChars(sFormat, pFormat);
    pEnv->ReleaseStringUTFChars(sOptions, pOptions);

    return result;
}

/* DirectBufferAllocator */

extern "C" SAL_JNI_EXPORT jobject JNICALL Java_org_libreoffice_kit_DirectBufferAllocator_allocateDirectBufferNative
    (JNIEnv* pEnv, jclass /*aClass*/, jint nSize)
{
    jobject aBuffer = NULL;

    if (nSize > 0)
    {
        void* pMemory = malloc(nSize);
        if (pMemory != NULL)
        {
            aBuffer = pEnv->NewDirectByteBuffer(pMemory, nSize);
            if (aBuffer == NULL)
            {
                free(pMemory);
            }
        }
    }
    return aBuffer;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_DirectBufferAllocator_freeDirectBufferNative
    (JNIEnv* pEnv, jclass, jobject aBuffer)
{
    free(pEnv->GetDirectBufferAddress(aBuffer));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
