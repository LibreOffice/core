/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

public class DocumentPartViewListAdapter extends ArrayAdapter<DocumentPartView> {
    private static final String LOGTAG = DocumentPartViewListAdapter.class.getSimpleName();

    private final Activity activity;
    private final ThumbnailCreator thumbnailCollector;

    public DocumentPartViewListAdapter(Activity activity, int resource, List<DocumentPartView> objects) {
        super(activity, resource, objects);
        this.activity = activity;
        this.thumbnailCollector = new ThumbnailCreator();
    }

    @Override
    public View getView(int position, View view, ViewGroup parent) {
        if (view == null) {
            LayoutInflater layoutInflater = activity.getLayoutInflater();
            view = layoutInflater.inflate(R.layout.document_part_list_layout, null);
        }

        DocumentPartView documentPartView = getItem(position);
        TextView textView = (TextView) view.findViewById(R.id.text);
        textView.setText(documentPartView.partName);

        ImageView imageView = (ImageView) view.findViewById(R.id.image);
        thumbnailCollector.createThumbnail(position, imageView);

        return view;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
