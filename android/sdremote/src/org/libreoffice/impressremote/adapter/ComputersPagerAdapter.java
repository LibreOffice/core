package org.libreoffice.impressremote.adapter;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import org.libreoffice.impressremote.fragment.ComputersFragment;

public class ComputersPagerAdapter extends FragmentPagerAdapter {
    private static final int PAGER_SIZE = 2;

    private static final class PagerPositions {
        private PagerPositions() {
        }

        public static final int BLUETOOTH = 0;
        public static final int WIFI = 1;
    }

    public ComputersPagerAdapter(FragmentManager aFragmentManager) {
        super(aFragmentManager);
    }

    @Override
    public Fragment getItem(int aPosition) {
        switch (aPosition) {
            case PagerPositions.BLUETOOTH:
                return ComputersFragment.newInstance(ComputersFragment.Type.BLUETOOTH);

            case PagerPositions.WIFI:
                return ComputersFragment.newInstance(ComputersFragment.Type.WIFI);

            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return PAGER_SIZE;
    }
}
