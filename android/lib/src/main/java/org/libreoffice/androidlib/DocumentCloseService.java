/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Keeps the process alive while a document is open, and while a closed document is still saved
 * and unloaded in the background, and lets a new activity wait until the document it opens is
 * no longer closing.
 *
 * It starts with the first document that loads, while the app is on screen, because a start from
 * the background is refused after a few seconds and a close from onDestroy can come that late.
 *
 * This is a started service without a foreground notification, so a process under memory
 * pressure can still be killed.
 */
public class DocumentCloseService extends Service {
    private static final String TAG = "DocumentCloseService";

    private static final Object sLock = new Object();
    private static final Handler sMainHandler = new Handler(Looper.getMainLooper());
    private static int sOpen = 0;
    private static int sClosing = 0;
    private static final Map<String, Integer> sClosingByUri = new HashMap<>();
    private static final Map<String, List<Runnable>> sWaiters = new HashMap<>();

    public static void documentOpened(Context context) {
        synchronized (sLock) {
            adjust(context, 1, 0);
        }
    }

    /** The URI may be null. */
    public static void pendingStarted(Context context, String uri) {
        synchronized (sLock) {
            adjust(context, -1, 1);
            if (uri != null)
                sClosingByUri.merge(uri, 1, Integer::sum);
        }
    }

    private static void adjust(Context context, int openDelta, int closingDelta) {
        final int before = sOpen + sClosing;
        sOpen += openDelta;
        sClosing += closingDelta;
        final int after = sOpen + sClosing;
        if (before == 0 && after > 0) {
            try {
                context.startService(new Intent(context, DocumentCloseService.class));
            } catch (IllegalStateException e) {
                Log.w(TAG, "could not start: " + e.getMessage());
            }
        } else if (before > 0 && after == 0) {
            context.stopService(new Intent(context, DocumentCloseService.class));
        }
    }

    /** Takes the URI given to pendingStarted. */
    public static void pendingFinished(Context context, String uri) {
        List<Runnable> waiters = null;
        synchronized (sLock) {
            if (uri != null) {
                Integer count = sClosingByUri.get(uri);
                if (count != null && count > 1) {
                    sClosingByUri.put(uri, count - 1);
                } else {
                    sClosingByUri.remove(uri);
                    if (sWaiters.containsKey(uri))
                        waiters = new ArrayList<>(sWaiters.get(uri));
                }
            }
            adjust(context, 0, -1);
        }
        if (waiters != null) {
            for (Runnable waiter : waiters)
                sMainHandler.post(waiter);
        }
    }

    public static boolean isPending(String uri) {
        synchronized (sLock) {
            return sClosingByUri.containsKey(uri);
        }
    }

    /**
     * Run the work on the main thread once no document with this URI is closing any more, or
     * when the timeout passes, whichever comes first.
     */
    public static void whenNotPending(String uri, long timeoutMs, Runnable work) {
        final Runnable waiter = new Runnable() {
            @Override
            public void run() {
                sMainHandler.removeCallbacks(this);
                synchronized (sLock) {
                    final List<Runnable> waiters = sWaiters.get(uri);
                    if (waiters == null || !waiters.remove(this))
                        return;

                    if (waiters.isEmpty())
                        sWaiters.remove(uri);
                    if (sClosingByUri.containsKey(uri))
                        Log.w(TAG, "gave up waiting for the close of " + uri);
                }
                work.run();
            }
        };
        synchronized (sLock) {
            if (!sClosingByUri.containsKey(uri)) {
                sMainHandler.post(work);
                return;
            }
            List<Runnable> waiters = sWaiters.get(uri);
            if (waiters == null) {
                waiters = new ArrayList<>();
                sWaiters.put(uri, waiters);
            }
            waiters.add(waiter);
        }
        sMainHandler.postDelayed(waiter, timeoutMs);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
