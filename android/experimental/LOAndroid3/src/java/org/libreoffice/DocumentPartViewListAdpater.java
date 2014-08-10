package org.libreoffice;

import android.app.Activity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

public class DocumentPartViewListAdpater extends ArrayAdapter<DocumentPartView> {
    private static final String LOGTAG = DocumentPartViewListAdpater.class.getSimpleName();

    private final Activity activity;

    public DocumentPartViewListAdpater(Activity activity, int resource, List<DocumentPartView> objects) {
        super(activity, resource, objects);
        this.activity = activity;
    }

    @Override
    public View getView(int position, View view, ViewGroup parent) {
        if (view == null) {
            LayoutInflater layoutInflater = activity.getLayoutInflater();
            view = layoutInflater.inflate(R.layout.document_part_list_layout, null);
        }

        DocumentPartView documentPartView = getItem(position);
        TextView textView = (TextView) view.findViewById(R.id.text);
        textView.setText(documentPartView.getPartName());
        Log.i(LOGTAG, "getView - " + documentPartView.getPartName());

        ImageView imageView = (ImageView) view.findViewById(R.id.image);
        imageView.setImageResource(R.drawable.writer);

        return view;
    }
}
