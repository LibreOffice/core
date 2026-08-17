/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <COKit/COKit.hxx>

/* COKit */

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
    const char* pTemp = pEnv->GetStringUTFChars(aJavaString, NULL);
    const char* pClone = strdup(pTemp);
    pEnv->ReleaseStringUTFChars(aJavaString, pTemp);

    return pClone;
}

} // anonymous namespace

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Office_getError
    (JNIEnv* pEnv, jobject aObject)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);
    std::string aError = pCOKit->getError();
    return pEnv->NewStringUTF(aError.c_str());
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_destroy
    (JNIEnv* pEnv, jobject aObject)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);
    delete pCOKit;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_destroyAndExit(JNIEnv* pEnv, jobject aObject)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);
    delete pCOKit;
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
static CallbackData gCallbackDataKit;

/**
 * Handle retrieved callback
 */
void messageCallback(COKitCallbackType eType, const char* pPayload, void* pData)
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
    aParameter[0].i = static_cast<jint>(eType);
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
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);

    COKitDocument* pDocument = pCOKit->documentLoad(aCloneDocumentPath);

    if (pDocument == NULL)
        return NULL;

    jobject aHandle = pEnv->NewDirectByteBuffer((void*) pDocument, sizeof(COKitDocument));

    return aHandle;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_setDocumentPassword
    (JNIEnv* pEnv, jobject aObject, jstring sUrl, jstring sPassword)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);

    char const* pUrl = copyJavaString(pEnv, sUrl);
    if (sPassword == NULL) {
        pCOKit->setDocumentPassword(pUrl, nullptr);
    } else {
        char const* pPassword = copyJavaString(pEnv, sPassword);
        pCOKit->setDocumentPassword(pUrl, pPassword);
    }
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_setOptionalFeatures
    (JNIEnv* pEnv, jobject aObject, jlong options)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);

    pCOKit->setOptionalFeatures(static_cast<COKitOptionalFeatures>(options));
}

/** Implementation of org.libreoffice.kit.Office.bindMessageCallback method */
extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Office_bindMessageCallback
    (JNIEnv* pEnv, jobject aObject)
{
    COKit* pCOKit = getHandle<COKit>(pEnv, aObject);

    gCallbackDataKit.aObject = (jobject) pEnv->NewGlobalRef(aObject);
    jclass aClass = pEnv->GetObjectClass(aObject);
    gCallbackDataKit.aClass = (jclass) pEnv->NewGlobalRef(aClass);

    gCallbackDataKit.aJavaCallbackMethod = pEnv->GetMethodID(aClass, "messageRetrievedLOKit", "(ILjava/lang/String;)V");

    pCOKit->registerCallback(messageCallback, (void*) &gCallbackDataKit);
}

/* Document */

/** Implementation of org.libreoffice.kit.Document.bindMessageCallback method */
extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_bindMessageCallback
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    gCallbackData.aObject = (jobject) pEnv->NewGlobalRef(aObject);
    jclass aClass = pEnv->GetObjectClass(aObject);
    gCallbackData.aClass = (jclass) pEnv->NewGlobalRef(aClass);

    gCallbackData.aJavaCallbackMethod = pEnv->GetMethodID(aClass, "messageRetrieved", "(ILjava/lang/String;)V");

    pDocument->registerCallback(messageCallback, (void*) &gCallbackData);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_destroy
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    delete pDocument;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setPart
    (JNIEnv* pEnv, jobject aObject, jint aPart)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->setPart(aPart);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getPart
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    return (jint) pDocument->getPart();
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Document_getPartPageRectangles
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    std::string pRectangles = pDocument->getWriterPageRectangles();
    return pEnv->NewStringUTF(pRectangles.c_str());
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getParts
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    return (jint) pDocument->getParts();
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Document_getPartName
    (JNIEnv* pEnv, jobject aObject, jint nPartIndex)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    char* pPartName = pDocument->getPartName(nPartIndex);
    return pEnv->NewStringUTF(pPartName);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setPartMode
    (JNIEnv* pEnv, jobject aObject, jint nPartMode)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    pDocument->setPartMode(static_cast<COKitPartMode>(nPartMode));
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_getDocumentTypeNative
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    return static_cast<jint>(pDocument->getDocumentType());
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_paintTileNative
    (JNIEnv* pEnv, jobject aObject, jobject aByteBuffer,
    jint nCanvasWidth, jint nCanvasHeight, jint nTilePosX, jint nTilePosY,
    jint nTileWidth, jint nTileHeight)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    unsigned char* buffer = (unsigned char*) pEnv->GetDirectBufferAddress(aByteBuffer);
    pDocument->paintTile(buffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

extern "C" SAL_JNI_EXPORT jlong JNICALL Java_org_libreoffice_kit_Document_getDocumentHeight
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    long nWidth;
    long nHeight;
    pDocument->getDocumentSize(&nWidth, &nHeight);
    return nHeight;
}

extern "C" SAL_JNI_EXPORT jlong JNICALL Java_org_libreoffice_kit_Document_getDocumentWidth
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    long nWidth;
    long nHeight;
    pDocument->getDocumentSize(&nWidth, &nHeight);
    return nWidth;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_initializeForRendering
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->initializeForRendering(NULL);
}

extern "C" SAL_JNI_EXPORT jint JNICALL Java_org_libreoffice_kit_Document_saveAs
    (JNIEnv* pEnv, jobject aObject, jstring sUrl, jstring sFormat, jstring sOptions)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    const char* pUrl = pEnv->GetStringUTFChars(sUrl, NULL);
    const char* pFormat = pEnv->GetStringUTFChars(sFormat, NULL);
    const char* pOptions = pEnv->GetStringUTFChars(sOptions, NULL);

    int result = pDocument->saveAs(pUrl, pFormat, pOptions);

    pEnv->ReleaseStringUTFChars(sUrl, pUrl);
    pEnv->ReleaseStringUTFChars(sFormat, pFormat);
    pEnv->ReleaseStringUTFChars(sOptions, pOptions);

    return result;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postKeyEvent
    (JNIEnv* pEnv, jobject aObject, jint nType, jint nCharCode, jint nKeyCode)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->postKeyEvent(static_cast<COKitKeyEventType>(nType), nCharCode,
                                    nKeyCode);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postMouseEvent
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y, jint count, jint button, jint modifier)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->postMouseEvent(static_cast<COKitMouseEventType>(type), x, y,
                                      count, button, modifier);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_postUnoCommand
    (JNIEnv* pEnv, jobject aObject, jstring command, jstring arguments, jboolean bNotifyWhenFinished)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    const char* pCommand = pEnv->GetStringUTFChars(command, NULL);
    const char* pArguments = nullptr;
    if (arguments != NULL)
        pArguments = pEnv->GetStringUTFChars(arguments, NULL);

    pDocument->postUnoCommand(pCommand, pArguments, bNotifyWhenFinished);

    pEnv->ReleaseStringUTFChars(command, pCommand);
    if (arguments != NULL)
        pEnv->ReleaseStringUTFChars(arguments, pArguments);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setTextSelection
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->setTextSelection(static_cast<COKitSetTextSelectionType>(type), x, y);
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Document_getTextSelection
    (JNIEnv* pEnv, jobject aObject, jstring mimeType)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    const char* pMimeType = pEnv->GetStringUTFChars(mimeType, NULL);

    std::string aSelection = pDocument->getTextSelection(pMimeType, nullptr);

    pEnv->ReleaseStringUTFChars(mimeType, pMimeType);

    return pEnv->NewStringUTF(aSelection.c_str());
}

extern "C" SAL_JNI_EXPORT jboolean JNICALL Java_org_libreoffice_kit_Document_paste
    (JNIEnv* pEnv, jobject aObject, jstring mimeType, jstring data)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    const char* pMimeType = pEnv->GetStringUTFChars(mimeType, NULL);
    const char* pData = pEnv->GetStringUTFChars(data, NULL);
    const size_t nSize = pEnv->GetStringLength(data);

    bool result = pDocument->paste(pMimeType, pData, nSize);
    pEnv->ReleaseStringUTFChars(mimeType, pMimeType);
    pEnv->ReleaseStringUTFChars(data, pData);

    return result;
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setGraphicSelection
    (JNIEnv* pEnv, jobject aObject, jint type, jint x, jint y)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->setGraphicSelection(static_cast<COKitSetGraphicSelectionType>(type), x, y);
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_resetSelection
    (JNIEnv* pEnv, jobject aObject)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->resetSelection();
}

extern "C" SAL_JNI_EXPORT jstring JNICALL Java_org_libreoffice_kit_Document_getCommandValues
    (JNIEnv* pEnv, jobject aObject, jstring command)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);

    const char* pCommand = pEnv->GetStringUTFChars(command, NULL);

    std::string pValue = pDocument->getCommandValues(pCommand);

    pEnv->ReleaseStringUTFChars(command, pCommand);

    return pEnv->NewStringUTF(pValue.c_str());
}

extern "C" SAL_JNI_EXPORT void JNICALL Java_org_libreoffice_kit_Document_setClientZoom
    (JNIEnv* pEnv, jobject aObject, jint nTilePixelWidth, jint nTilePixelHeight, jint nTileTwipWidth, jint nTileTwipHeight)
{
    COKitDocument* pDocument = getHandle<COKitDocument>(pEnv, aObject);
    pDocument->setClientZoom(nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
