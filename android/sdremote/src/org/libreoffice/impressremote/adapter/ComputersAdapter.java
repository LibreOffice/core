/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.adapter;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.Server;

public class ComputersAdapter extends ArrayAdapter<Server> {
    private final LayoutInflater mLayoutInflater;

    public ComputersAdapter(Context aContext) {
        super(aContext, R.layout.list_item);

        mLayoutInflater = LayoutInflater.from(aContext);
    }

    @Override
    public View getView(int aPosition, View aConvertView, ViewGroup aParentViewGroup) {
        TextView aListItem = (TextView) getView(aConvertView, aParentViewGroup);

        aListItem.setText(buildListItemText(aPosition));

        return aListItem;
    }

    private View getView(View aConvertView, ViewGroup aParentViewGroup) {
        if (aConvertView != null) {
            return aConvertView;
        }

        return mLayoutInflater.inflate(R.layout.list_item, aParentViewGroup, false);
    }

    private String buildListItemText(int aPosition) {
        return getItem(aPosition).getName();
    }

    public void add(List<Server> aServers) {
        setNotifyOnChange(false);

        for (Server aServer : aServers) {
            add(aServer);
        }

        notifyDataSetChanged();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
