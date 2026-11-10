/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <jni.h>

#include <string>

struct COKit;

extern int coolwsd_server_socket_fd;

extern const char* user_name;

/** Deliver a message from the native code to the activity of the document with this number. */
void postDirectMessage(unsigned documentId, const std::string& message);

/** Report to the activity of the document with this number that the engine has dropped the
    document, and let go of that activity. */
void documentClosed(unsigned documentId);

/**
 * The JNIEnv of the current thread for one scope. The engine attaches the kit thread while a
 * document runs, so there this is a lookup. A thread that is not attached, such as the
 * forwarding thread, is attached here and detached again at the end of the scope. The local
 * references made in the scope are released at its end, as a native thread never returns to
 * Java to release them.
 */
class JNIThreadContext
{
    JNIEnv* _env = nullptr;
    bool _attachedHere = false;
    bool _framePushed = false;

public:
    explicit JNIThreadContext(jint localCapacity = 16);
    ~JNIThreadContext();

    JNIThreadContext(const JNIThreadContext&) = delete;
    JNIThreadContext& operator=(const JNIThreadContext&) = delete;

    /// null when the thread could not be attached
    JNIEnv* getEnv() const { return _framePushed ? _env : nullptr; }
};

/** Resolve the SystemClipboard Java class and its methods; needs a thread the JVM created. */
void resolveSystemClipboardClass(JNIEnv* env);

/** Install the process-global clipboard provider backed by SystemClipboard.java. */
void install_clipboard_provider(COKit& rOffice);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
