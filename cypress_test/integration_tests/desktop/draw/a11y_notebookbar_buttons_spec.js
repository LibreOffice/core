/* global require */

const sweep = require('../../common/a11y_notebookbar_sweep');

// No tab of the Draw notebookbar shows a gallery.
sweep.sweepNotebookbar('Draw', 'draw/insert_position.fodg', { galleries: false });
