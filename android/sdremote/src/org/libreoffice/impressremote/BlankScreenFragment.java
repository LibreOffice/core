package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageView;

public class BlankScreenFragment extends Fragment {

    CommunicationService mCommunicationService;

    public BlankScreenFragment(CommunicationService aCommunicationService) {
        mCommunicationService = aCommunicationService;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                    Bundle savedInstanceState) {

        View v = inflater.inflate(R.layout.fragment_blankscreen, container,
                        false);

        Bitmap aBitmap = mCommunicationService.getSlideShow().getImage(
                        mCommunicationService.getSlideShow().getCurrentSlide());

        // Process the image
        final int borderWidth = 8;

        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setShadowLayer(borderWidth, 0, 0, Color.BLACK);

        RectF aRect = new RectF(borderWidth, borderWidth, borderWidth
                        + aBitmap.getWidth(), borderWidth + aBitmap.getHeight());
        Bitmap aOut = Bitmap.createBitmap(aBitmap.getWidth() + 2 * borderWidth,
                        aBitmap.getHeight() + 2 * borderWidth,
                        aBitmap.getConfig());
        Canvas canvas = new Canvas(aOut);
        canvas.drawColor(Color.TRANSPARENT);
        canvas.drawRect(aRect, p);
        canvas.drawBitmap(aBitmap, null, aRect, null);

        ImageView aImage = (ImageView) v
                        .findViewById(R.id.blankscreen_slidepreview);
        aImage.setImageBitmap(aOut);

        OnClickListener aListener = new OnClickListener() {

            @Override
            public void onClick(View v) {
                getFragmentManager().popBackStackImmediate();
            }

        };

        v.findViewById(R.id.blankscreen_slidepreview).setOnClickListener(
                        aListener);
        v.findViewById(R.id.blankscreen_return).setOnClickListener(aListener);
        mCommunicationService.getTransmitter().blankScreen();
        // TODO Auto-generated method stub
        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mCommunicationService.getTransmitter().resume();
    }
}
