# Welcome slideshow

The slideshow that Collabora Office shows at the first start and after an
update, and from Help > Latest Updates. Every desktop app build bundles it as
`welcome/welcome-slideshow.odp`, translated at build time from `po/` with the
introdocs tooling (`../introdocs/create_l10n_intro_docs.py`). `welcome.svg` is
shown while the slideshow loads. Debug builds show it only with the
`welcome.enable` setting.

The translations are maintained on Weblate (project collabora-online,
component welcome-presentation).

## Adding strings

Prefix every translatable paragraph in `welcome-slideshow.odp` with "_", for
example "_This looks beautiful", one marker per paragraph (see
`../introdocs/l10n_odf.py` for the rules). Then update the template and the
translations:

    browser/introdocs/update_pot_and_pos.sh browser/welcome-slideshow

`make check-introdocs` in `browser/` verifies that the template is current.
