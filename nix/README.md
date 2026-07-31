# Nix

This is a continuation of https://github.com/CollaboraOnline/nix-build-support/,
now in the monorepo for easier access and the ability to update along with other
patches.

Unlike `nix-build-support`, we're using neither `nilla` nor `flakes`. This is
(1) to avoid pulling in an extra dependency and (2) because flakes copy the
git repo they are used in to the nix store, which is untenable with a monorepo
the size of Collabora Online.

## Entering shells

To leave open the option of adding package builds with nix, shells are stored
under the `shells` option. They are not separated by system (we enter the shell
for the current system) so to enter a given shell you only need to provide its
name. For example, to enter the ios shell you would run:

```
nix-shell . -A shells.ios
```

Our shells provide all the tools needed to build Collabora Online, so you can
also use them in pure mode. This is mostly intended for CI, as you likely have
some development tools (e.g. ripgrep, your editor, etc.) which are not installed
in the shells

```
nix-shell . -A shells.ios --pure
```

## All shells

| Shell | Description                                     |
| ----- | ----------------------------------------------- |
| ios   | Used for building the Collabora Office iOS apps |
