/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.ui;

import android.support.v4.content.ContextCompat;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.libreoffice.R;

import java.util.List;

class RecentFilesAdapter extends RecyclerView.Adapter<RecentFilesAdapter.ViewHolder> {

    private LibreOfficeUIActivity mActivity;
    private List<RecentFile> recentFiles;

    RecentFilesAdapter(LibreOfficeUIActivity activity, List<RecentFile> recentFiles) {
        this.mActivity = activity;
        this.recentFiles = recentFiles;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View item = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_recent_files, parent, false);
        return new ViewHolder(item);
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        final RecentFile entry = recentFiles.get(position);

        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mActivity.openDocument(entry.getUri());
            }
        });

        final String filename = entry.getDisplayName();
        holder.textView.setText(filename);

        int compoundDrawableInt = 0;

        switch (FileUtilities.getType(filename)) {
            case FileUtilities.DOC:
                compoundDrawableInt = R.drawable.writer;
                break;
            case FileUtilities.CALC:
                compoundDrawableInt = R.drawable.calc;
                break;
            case FileUtilities.DRAWING:
                compoundDrawableInt = R.drawable.draw;
                break;
            case FileUtilities.IMPRESS:
                compoundDrawableInt = R.drawable.impress;
                break;
        }

        // set icon if known filetype was detected
        if (compoundDrawableInt != 0)
            holder.imageView.setImageDrawable(ContextCompat.getDrawable(mActivity, compoundDrawableInt));
    }

    @Override
    public int getItemCount() {
        return recentFiles.size();
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        TextView textView;
        ImageView imageView;

        ViewHolder(View itemView) {
            super(itemView);
            this.textView = itemView.findViewById(R.id.textView);
            this.imageView = itemView.findViewById(R.id.imageView);
        }
    }
}
