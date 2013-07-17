package org.libreoffice.impressremote.activity;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.fragment.SlidesGridFragment;
import org.libreoffice.impressremote.fragment.SlidesPagerFragment;

public class SlideShowActivity extends SherlockFragmentActivity {
    private static enum Mode {
        PAGER, GRID
    }

    private Mode mMode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mMode = Mode.PAGER;

        setUpHomeButton();

        setUpFragment();
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpFragment() {
        switch (mMode) {
            case PAGER:
                setUpFragment(SlidesPagerFragment.newInstance());
                break;

            case GRID:
                setUpFragment(SlidesGridFragment.newInstance());
                break;

            default:
                setUpFragment(SlidesPagerFragment.newInstance());
                break;
        }
    }

    private void setUpFragment(Fragment aFragment) {
        FragmentTransaction aTransaction = getSupportFragmentManager().beginTransaction();

        aTransaction.replace(android.R.id.content, aFragment);

        aTransaction.commit();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu aMenu) {
        getSupportMenuInflater().inflate(getActionBarMenuResourceId(), aMenu);

        return true;
    }

    private int getActionBarMenuResourceId() {
        switch (mMode) {
            case PAGER:
                return R.menu.menu_action_bar_slide_show_pager;

            case GRID:
                return R.menu.menu_action_bar_slide_show_grid;

            default:
                return R.menu.menu_action_bar_slide_show_pager;
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_slides_grid:
                mMode = Mode.GRID;

                setUpFragment();
                refreshActionBarMenu();

                return true;

            case R.id.menu_slides_pager:
                mMode = Mode.PAGER;

                setUpFragment();
                refreshActionBarMenu();

                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void refreshActionBarMenu() {
        supportInvalidateOptionsMenu();
    }
}
