#!/usr/bin/env bash
#
# clone-online.sh
#
# Standalone helper that clones the Collabora Online monorepo from
# gerrit.collaboraoffice.com over SSH. It walks the user through
# generating an SSH key, registering it in Gerrit, cloning the repo,
# and (optionally) running the two-step engine + online build.
#
# Usage:
#   ./clone-online.sh [target-directory] [--build|--no-build]
#                                        [--skip-engine] [--skip-online]
#
# If target-directory is omitted, it defaults to ./online in the
# current working directory.
#
# Build flags:
#   --build         Always run the build, no prompt.
#   --no-build      Never build; exit after cloning.
#   --skip-engine   Skip the engine (core) build stage.
#   --skip-online   Skip the online build stage.
#
# With no build flag, the script asks "Build now? (Y/n)" after cloning.

set -eu

GERRIT_HOST="gerrit.collaboraoffice.com"
GERRIT_PORT="29418"
GERRIT_REPO="online"
GERRIT_SETTINGS_URL="https://gerrit.collaboraoffice.com/settings/#SSHKeys"

TARGET_DIR=""
BUILD_MODE="ask"      # ask | yes | no
SKIP_ENGINE=0
SKIP_ONLINE=0

for arg in "$@"; do
    case "$arg" in
        --build)        BUILD_MODE="yes" ;;
        --no-build)     BUILD_MODE="no" ;;
        --skip-engine)    SKIP_ENGINE=1 ;;
        --skip-online)  SKIP_ONLINE=1 ;;
        --help|-h)
            sed -n '2,21p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        --*)
            printf 'Unknown option: %s\n' "$arg" >&2
            exit 2
            ;;
        *)
            if [ -n "$TARGET_DIR" ]; then
                printf 'Multiple target directories given: %s and %s\n' \
                    "$TARGET_DIR" "$arg" >&2
                exit 2
            fi
            TARGET_DIR="$arg"
            ;;
    esac
done

if [ -z "$TARGET_DIR" ]; then
    TARGET_DIR="online"
fi

say() {
    printf '\n=== %s ===\n' "$1"
}

info() {
    printf '    %s\n' "$1"
}

err() {
    printf '\n!!! %s\n' "$1" >&2
}

prompt() {
    # prompt VAR "Question text"
    local __var="$1"
    local __msg="$2"
    local __reply=""
    while :; do
        printf '%s ' "$__msg"
        IFS= read -r __reply || true
        # Strip leading and trailing whitespace.
        __reply="${__reply#"${__reply%%[![:space:]]*}"}"
        __reply="${__reply%"${__reply##*[![:space:]]}"}"
        if [ -n "$__reply" ]; then
            break
        fi
        err "Empty input. Please type a value and press Enter."
    done
    printf -v "$__var" '%s' "$__reply"
}

prompt_with_default() {
    # prompt_with_default VAR "Question text" "default value"
    local __var="$1"
    local __msg="$2"
    local __default="$3"
    local __reply=""
    printf '%s [%s] ' "$__msg" "$__default"
    IFS= read -r __reply || true
    if [ -z "$__reply" ]; then
        __reply="$__default"
    fi
    printf -v "$__var" '%s' "$__reply"
}

# ---------------------------------------------------------------------------
# Step 0: Pre-flight checks
# ---------------------------------------------------------------------------

check_command() {
    local cmd="$1"
    if ! command -v "$cmd" >/dev/null 2>&1; then
        err "Required command not found: $cmd"
        info "Install it and try again. Hints:"
        info "  Linux (Debian/Ubuntu): sudo apt install git openssh-client"
        info "  Linux (Fedora):        sudo dnf install git openssh-clients"
        info "  macOS:                 brew install git  (ssh ships with macOS)"
        exit 1
    fi
}

say "Step 0: Checking required tools"
check_command git
check_command ssh
check_command ssh-keygen
info "git, ssh, and ssh-keygen are available."

# ---------------------------------------------------------------------------
# Step 1: Ask for Gerrit username
# ---------------------------------------------------------------------------

say "Step 1: Your Gerrit username"
info "This is the username shown on your Gerrit profile page."
info "Example: Darshan-upadhyay1110"
prompt GERRIT_USER "Gerrit username:"

# ---------------------------------------------------------------------------
# Step 2: Detect or create an SSH key
# ---------------------------------------------------------------------------

say "Step 2: Looking for an SSH key on this computer"

SSH_DIR="$HOME/.ssh"
PUB_KEY=""
PRIV_KEY=""

if [ -f "$SSH_DIR/id_ed25519.pub" ]; then
    PUB_KEY="$SSH_DIR/id_ed25519.pub"
    PRIV_KEY="$SSH_DIR/id_ed25519"
    info "Found existing key: $PUB_KEY"
elif [ -f "$SSH_DIR/id_rsa.pub" ]; then
    PUB_KEY="$SSH_DIR/id_rsa.pub"
    PRIV_KEY="$SSH_DIR/id_rsa"
    info "Found existing key: $PUB_KEY"
else
    info "No SSH key found. We'll create a new one."

    # Refuse to overwrite an existing private key whose .pub is missing.
    if [ -e "$SSH_DIR/id_ed25519" ]; then
        err "Found $SSH_DIR/id_ed25519 but no matching .pub file."
        info "Refusing to overwrite. Please move or remove that file and re-run."
        exit 1
    fi

    DEFAULT_EMAIL="$(whoami)@$(hostname)"
    prompt_with_default KEY_EMAIL \
        "Email to label the key with (just hit Enter for default):" \
        "$DEFAULT_EMAIL"

    mkdir -p "$SSH_DIR"
    chmod 700 "$SSH_DIR"

    info "Generating an ed25519 SSH key (no passphrase, for simplicity)."
    info "If you want a stronger key with a passphrase later, run:"
    info "  ssh-keygen -p -f $SSH_DIR/id_ed25519"
    ssh-keygen -t ed25519 -C "$KEY_EMAIL" -f "$SSH_DIR/id_ed25519" -N "" >/dev/null

    PUB_KEY="$SSH_DIR/id_ed25519.pub"
    PRIV_KEY="$SSH_DIR/id_ed25519"
    info "Key created: $PUB_KEY"
fi

# ---------------------------------------------------------------------------
# Step 3: Test SSH auth against Gerrit
# ---------------------------------------------------------------------------

test_gerrit_ssh() {
    local out
    out=$(ssh -p "$GERRIT_PORT" \
              -o BatchMode=yes \
              -o StrictHostKeyChecking=accept-new \
              -o ConnectTimeout=10 \
              "${GERRIT_USER}@${GERRIT_HOST}" 2>&1) || true
    # Gerrit prints something like:
    #   ****    Welcome to Gerrit Code Review    ****
    #   Hi <user>, you have successfully connected over SSH.
    case "$out" in
        *"Welcome to Gerrit"*|*"successfully connected"*)
            return 0
            ;;
    esac
    # Save last output for diagnostics.
    LAST_SSH_OUTPUT="$out"
    return 1
}

say "Step 3: Testing SSH connection to Gerrit"
LAST_SSH_OUTPUT=""
if test_gerrit_ssh; then
    info "SSH key already works with Gerrit. Skipping registration."
    SSH_OK=1
else
    info "Could not authenticate yet. We'll register your key with Gerrit."
    SSH_OK=0
fi

# ---------------------------------------------------------------------------
# Step 4: Walk the user through registering the key in Gerrit
# ---------------------------------------------------------------------------

copy_to_clipboard() {
    local file="$1"
    if command -v pbcopy >/dev/null 2>&1; then
        pbcopy < "$file" && return 0
    elif [ -n "${WAYLAND_DISPLAY:-}" ] && command -v wl-copy >/dev/null 2>&1; then
        wl-copy < "$file" && return 0
    elif command -v xclip >/dev/null 2>&1; then
        xclip -selection clipboard < "$file" && return 0
    elif command -v xsel >/dev/null 2>&1; then
        xsel --clipboard --input < "$file" && return 0
    fi
    return 1
}

if [ "$SSH_OK" -eq 0 ]; then
    say "Step 4: Add your public key to Gerrit"

    info "Below is your PUBLIC key. It is safe to share. Copy ALL of it."
    printf '\n--- COPY EVERYTHING BETWEEN THESE LINES ---\n'
    cat "$PUB_KEY"
    printf -- '--- END ---\n\n'

    if copy_to_clipboard "$PUB_KEY"; then
        info "(The key has also been copied to your clipboard.)"
    fi

    info "Now do this in your web browser:"
    info "  1. Open this URL:"
    info "     $GERRIT_SETTINGS_URL"
    info "  2. Sign in with your Collabora account if prompted."
    info "  3. The page should jump to the 'SSH Keys' section."
    info "  4. Paste the key (Ctrl+V or Cmd+V) into the 'New SSH key' box."
    info "  5. Click the 'ADD NEW SSH KEY' button."

    printf '\nPress Enter once you have added the key in the browser... '
    IFS= read -r _ || true

    say "Step 3 (retry): Testing SSH connection to Gerrit"
    if test_gerrit_ssh; then
        info "Success. Gerrit recognises your key."
        SSH_OK=1
    else
        err "Still cannot authenticate with Gerrit."
        info "Possible causes:"
        info "  - The Gerrit username '$GERRIT_USER' is wrong."
        info "  - The key was not actually saved (re-check the Gerrit page)."
        info "  - Your network or firewall is blocking outbound port $GERRIT_PORT."
        if [ -n "$LAST_SSH_OUTPUT" ]; then
            info "Last SSH output:"
            printf '%s\n' "$LAST_SSH_OUTPUT" | sed 's/^/      /'
        fi
        info "Fix the issue and re-run this script."
        exit 1
    fi
fi

# ---------------------------------------------------------------------------
# Step 5: Clone the repo
# ---------------------------------------------------------------------------

say "Step 5: Cloning the monorepo"

CLONE_SKIPPED=0
if [ -e "$TARGET_DIR" ]; then
    if [ -d "$TARGET_DIR" ] && [ -z "$(ls -A "$TARGET_DIR" 2>/dev/null)" ]; then
        info "Target directory '$TARGET_DIR' exists and is empty. Cloning into it."
    else
        info "Target '$TARGET_DIR' already exists and is not empty."
        info "Assuming it's already a clone. Skipping the clone step."
        CLONE_SKIPPED=1
    fi
fi

if [ "$CLONE_SKIPPED" -eq 0 ]; then
    CLONE_URL="ssh://${GERRIT_USER}@${GERRIT_HOST}:${GERRIT_PORT}/${GERRIT_REPO}"
    info "Running: git clone $CLONE_URL $TARGET_DIR"
    if ! git clone "$CLONE_URL" "$TARGET_DIR"; then
        err "git clone failed."
        info "Check the output above. If it's a network issue, try again."
        exit 1
    fi
fi

info "Repo location: $TARGET_DIR"
info "The document engine (core) lives at: $TARGET_DIR/engine"
info "Build instructions: $TARGET_DIR/docs/building.md"

# ---------------------------------------------------------------------------
# Step 6: Decide whether to build
# ---------------------------------------------------------------------------

say "Step 6: Build the project?"

if [ "$BUILD_MODE" = "no" ]; then
    info "--no-build was passed. Stopping here."
    info "To build later, re-run this script with --build, or follow"
    info "$TARGET_DIR/docs/building.md manually."
    exit 0
fi

if [ "$BUILD_MODE" = "ask" ]; then
    info "A full build takes roughly 1-3 hours and uses ~30 GB of disk."
    info "The engine (core) is built first, then online on top of it."
    printf 'Build now? [Y/n] '
    BUILD_ANSWER=""
    IFS= read -r BUILD_ANSWER || true
    case "$BUILD_ANSWER" in
        ""|y|Y|yes|YES|Yes)
            BUILD_MODE="yes"
            ;;
        *)
            info "Skipping build. Re-run with --build when you're ready."
            exit 0
            ;;
    esac
fi

# Detect parallelism (portable across Linux and macOS).
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    JOBS=4
fi
info "Will use -j$JOBS for make."

# Resolve absolute paths for the engine and target dirs. Configure
# stores these in config files; relative paths break later when
# things run from other working dirs.
ENGINE_DIR_REL="$TARGET_DIR/engine"
if [ ! -d "$ENGINE_DIR_REL" ]; then
    err "Expected '$ENGINE_DIR_REL' to exist."
    info "This script only supports the co-26.04+ monorepo layout"
    info "(core grafted as the engine/ subdirectory)."
    exit 1
fi
ENGINE_DIR=$(cd "$ENGINE_DIR_REL" && pwd)
ONLINE_DIR=$(cd "$TARGET_DIR" && pwd)

# ---------------------------------------------------------------------------
# Step 7: Build the engine (core)
# ---------------------------------------------------------------------------

if [ "$SKIP_ENGINE" -eq 1 ]; then
    say "Step 7: Engine build (skipped via --skip-engine)"
else
    say "Step 7: Building the engine (core)"
    info "Working in: $ENGINE_DIR"
    info "If autogen fails, you likely have missing system packages."
    info "See $ENGINE_DIR/README* for the dependency list."

    (
        cd "$ENGINE_DIR"
        ./autogen.sh --without-system-nss \
            '--with-lang=de ja ar en-US en-GB en-ZA br' \
            --enable-dbgutil
    ) || {
        err "Engine autogen.sh failed."
        info "Read the error output above and install any missing packages."
        exit 1
    }

    (
        cd "$ENGINE_DIR"
        make -j"$JOBS"
    ) || {
        err "Engine 'make' failed."
        info "Read the error output above. Common causes: missing system"
        info "packages, out of disk space, or a flaky network for tarball"
        info "downloads. Re-run this script with --skip-engine to skip the"
        info "engine build once it's complete."
        exit 1
    }

    info "Engine build complete: $ENGINE_DIR/instdir"
fi

# ---------------------------------------------------------------------------
# Step 8: Build online
# ---------------------------------------------------------------------------

if [ "$SKIP_ONLINE" -eq 1 ]; then
    say "Step 8: Online build (skipped via --skip-online)"
else
    say "Step 8: Building online"
    info "Working in: $ONLINE_DIR"

    (
        cd "$ONLINE_DIR"
        ./autogen.sh
    ) || {
        err "Online autogen.sh failed."
        exit 1
    }

    (
        cd "$ONLINE_DIR"
        ./configure --enable-silent-rules \
            --with-lokit-path="$ENGINE_DIR/include" \
            --with-lo-path="$ENGINE_DIR/instdir" \
            --enable-debug --enable-cypress
    ) || {
        err "Online ./configure failed."
        info "Read the error output above. Make sure the engine has been"
        info "built first (instdir/ should exist under $ENGINE_DIR)."
        exit 1
    }

    (
        cd "$ONLINE_DIR"
        make -j"$JOBS"
    ) || {
        err "Online 'make' failed."
        exit 1
    }

    info "Online build complete."
fi

say "Done"
info "Repo:   $ONLINE_DIR"
info "Engine: $ENGINE_DIR"
info "See $ONLINE_DIR/docs/building.md for how to run coolwsd."
