/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.libreoffice.kit;

//
// We must manually allocate direct buffers in JNI to work around a bug where Honeycomb's
// ByteBuffer.allocateDirect() grossly overallocates the direct buffer size.
// https://code.google.com/p/android/issues/detail?id=16941
//

import android.util.Log;

import java.nio.ByteBuffer;

public final class DirectBufferAllocator {

    private static final String LOGTAG = DirectBufferAllocator.class.getSimpleName();

    private DirectBufferAllocator() {
    }

    private static native ByteBuffer allocateDirectBufferNative(int size);

    private static native void freeDirectBufferNative(ByteBuffer aBuffer);

    public static ByteBuffer allocate(int size) {
        return allocateVM(size);
    }

    public static ByteBuffer free(ByteBuffer buffer) {
        return freeVM(buffer);
    }

    private static ByteBuffer allocateJNI(int size) {
        ByteBuffer directBuffer = allocateDirectBufferNative(size);
        if (directBuffer == null) {
            if (size <= 0) {
                throw new IllegalArgumentException("Invalid allocation size: " + size);
            } else {
                throw new OutOfMemoryError("allocateDirectBuffer() returned null");
            }
        } else if (!directBuffer.isDirect()) {
            throw new AssertionError("allocateDirectBuffer() did not return a direct buffer");
        }
        return directBuffer;
    }

    private static ByteBuffer freeJNI(ByteBuffer buffer) {
        if (buffer == null) {
            return null;
        }

        if (!buffer.isDirect()) {
            throw new IllegalArgumentException("ByteBuffer must be direct");
        }

        freeDirectBufferNative(buffer);
        return null;
    }

    private static ByteBuffer allocateVM(int size) {
        ByteBuffer directBuffer = ByteBuffer.allocateDirect(size);
        if (directBuffer == null) {
            if (size <= 0) {
                throw new IllegalArgumentException("Invalid allocation size: " + size);
            } else {
                throw new OutOfMemoryError("allocateDirectBuffer() returned null");
            }
        } else if (!directBuffer.isDirect()) {
            throw new AssertionError("allocateDirectBuffer() did not return a direct buffer");
        }

        return directBuffer;
    }

    private static ByteBuffer freeVM(ByteBuffer buffer) {
        if (buffer == null) {
            return null;
        }

        if (!buffer.isDirect()) {
            throw new IllegalArgumentException("ByteBuffer must be direct");
        }
        // can't free buffer - leave this to the VM
        return null;
    }
}
