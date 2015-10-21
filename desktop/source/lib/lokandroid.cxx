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
#include <vcl/event.hxx>

#include <android/log.h>

#include <osl/detail/android-bootstrap.h>

#define LOK_USE_UNSTABLE_API

#include <LibreOfficeKit/LibreOfficeKit.h>

/* LibreOfficeKit */

namespace
{

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

} // anonymous namespace

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Office_getError
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);
    char* pError = pLibreOfficeKit->pClass->getError(pLibreOfficeKit);
    return pEnv->NewStringUTF(pError);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_destroy
    (JNIEnv* pEnv, jobject aObject)
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

namespace
{

struct CallbackData
{
    jmethodID aJavaCallbackMethod;
    jclass aClass;
    jobject aObject;
};

static CallbackData gCallbackData;

/**
 * Handle retrieved callback
 */
void messageCallback(int nType, const char* pPayload, void* pData)
{
    CallbackData* pCallbackData = (CallbackData*) pData;

    JavaVM* pJavaVM = lo_get_javavm();
    JNIEnv* pEnv;
    bool bIsAttached = false;

    int status = pJavaVM->GetEnv((void **) &pEnv, JNI_VERSION_1_6);

    if(status < 0)
    {
        status = pJavaVM->AttachCurrentThread(&pEnv, NULL);
        if(status < 0)
        {
            return;
        }
        bIsAttached = true;
    }

    jstring sPayload = pEnv->NewStringUTF(pPayload);

    jvalue aParameter[2];
    aParameter[0].i = nType;
    aParameter[1].l = sPayload;

    pEnv->CallVoidMethodA(pCallbackData->aObject, pCallbackData->aJavaCallbackMethod, aParameter);

    pEnv->DeleteLocalRef(sPayload);

    if (bIsAttached)
    {
        pJavaVM->DetachCurrentThread();
    }
}

} // anonymous namespace

extern "C" SAL_JNI_EXPORT jobject JNICALL Java_org_libreoffice_kit_Office_documentLoadNative
    (JNIEnv* pEnv, jobject aObject, jstring documentPath)
{
    const char* aCloneDocumentPath = copyJavaString(pEnv, documentPath);
    LibreOfficeKit* pLibreOfficeKit = getHandle<LibreOfficeKit>(pEnv, aObject);

    LibreOfficeKitDocument* pDocument = pLibreOfficeKit->pClass->documentLoad(pLibreOfficeKit, aCloneDocumentPath);

    if (pDocument == NULL)
        return NULL;

    jobject aHandle = pEnv->NewDirectByteBuffer((void*) pDocument, sizeof(LibreOfficeKitDocument));

    return aHandle;
}

/* Document */

/** Implementation of org.libreoffice.kit.Document.bindMessageCallback method */
extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_bindMessageCallback
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);

    gCallbackData.aObject = (jobject) pEnv->NewGlobalRef(aObject);
    jclass aClass = pEnv->GetObjectClass(aObject);
    gCallbackData.aClass = (jclass) pEnv->NewGlobalRef(aClass);

    gCallbackData.aJavaCallbackMethod = pEnv->GetMethodID(aClass, "messageRetrieved", "(ILjava/lang/String;)V");

    pDocument->pClass->registerCallback(pDocument, messageCallback, (void*) &gCallbackData);
}

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

    pDocument->pClass->setPartMode(pDocument, nPartMode);
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
    pDocument->pClass->paintTile(pDocument, buffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
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

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postKeyEvent
    (JNIEnv* pEnv, jobject aObject, jint nType, jint nCharCode, jint nKeyCode)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->postKeyEvent(pDocument, nType, nCharCode, nKeyCode);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postMouseEvent
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y, jint count)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->postMouseEvent(pDocument, type, x, y, count, MOUSE_LEFT, 0);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postUnoCommand
    (JNIEnv* pEnv, jobject aObject, jstring command)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);

    const char* pCommand = pEnv->GetStringUTFChars(command, NULL);

    pDocument->pClass->postUnoCommand(pDocument, pCommand, 0);

    pEnv->ReleaseStringUTFChars(command, pCommand);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setTextSelection
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->setTextSelection(pDocument, type, x, y);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setGraphicSelection
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->setGraphicSelection(pDocument, type, x, y);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_resetSelection
    (JNIEnv* pEnv, jobject aObject)
{
    LibreOfficeKitDocument* pDocument = getHandle<LibreOfficeKitDocument>(pEnv, aObject);
    pDocument->pClass->resetSelection(pDocument);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
